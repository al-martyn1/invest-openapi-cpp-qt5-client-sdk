/*! \file
    \brief Configs lookup test

 */

#include <iostream>
#include <exception>
#include <stdexcept>

#include <QCoreApplication>
#include <QString>
#include <QSettings>
#include <QTest>
#include <QDir>
#include <QElapsedTimer>

#include "invest_openapi/config_helpers.h"
#include "invest_openapi/api_config.h"
#include "invest_openapi/auth_config.h"
#include "invest_openapi/currencies_config.h"
#include "invest_openapi/database_config.h"


#include "invest_openapi/invest_openapi.h"
#include "invest_openapi/factory.h"
#include "invest_openapi/openapi_completable_future.h"
#include "invest_openapi/i_database_manager.h"
#include "invest_openapi/database_manager.h"

#include "invest_openapi/model_to_strings.h"



INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("_tkf_update_instruments");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;


    cout<<"Path to exe   : "<<QCoreApplication::applicationDirPath().toStdString()<<endl;

    cout << endl;

    namespace tkf=invest_openapi;
    using tkf::config_helpers::lookupForConfigFile;
    using tkf::config_helpers::FileReadable;

    QStringList lookupConfSubfolders = QString("conf;config").split( ';', Qt::SkipEmptyParts );


    auto dbConfigFullFileName    = lookupForConfigFile( "database.properties", lookupConfSubfolders, FileReadable() );
    auto logConfigFullFileName   = lookupForConfigFile( "logging.properties" , lookupConfSubfolders, FileReadable() );
    auto apiConfigFullFileName   = lookupForConfigFile( "config.properties"  , lookupConfSubfolders, FileReadable() );
    auto authConfigFullFileName  = lookupForConfigFile( "auth.properties"    , lookupConfSubfolders, FileReadable() );

    qDebug().nospace().noquote() << "DB   Config File: "<< dbConfigFullFileName   ;
    qDebug().nospace().noquote() << "Log  Config File: "<< logConfigFullFileName  ;
    qDebug().nospace().noquote() << "API  Config File: "<< apiConfigFullFileName  ;
    qDebug().nospace().noquote() << "Auth Config File: "<< authConfigFullFileName ;


    QSharedPointer<tkf::DatabaseConfig> pDatabaseConfig = QSharedPointer<tkf::DatabaseConfig>( new tkf::DatabaseConfig(dbConfigFullFileName, tkf::DatabasePlacementStrategyDefault()) );
    QSharedPointer<tkf::LoggingConfig>  pLoggingConfig  = QSharedPointer<tkf::LoggingConfig> ( new tkf::LoggingConfig(logConfigFullFileName) );

    qDebug().nospace().noquote() << "DB name: " << pDatabaseConfig->dbFilename;

    QSharedPointer<QSqlDatabase> pSqlDb = QSharedPointer<QSqlDatabase>( new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE")) );
    pSqlDb->setDatabaseName( pDatabaseConfig->dbFilename );

    if (!pSqlDb->open())
    {
        qDebug() << pSqlDb->lastError().text();
        return 0;
    }

    QSharedPointer<tkf::IDatabaseManager> pDbMan = tkf::createDatabaseManager( pSqlDb, pDatabaseConfig, pLoggingConfig );
    pDbMan->applyDefDecimalFromConfig( *pDatabaseConfig );


    auto apiConfig     = tkf::ApiConfig    ( apiConfigFullFileName  );
    auto authConfig    = tkf::AuthConfig   ( authConfigFullFileName );
    auto loggingConfig = tkf::LoggingConfig( logConfigFullFileName  );

    QSharedPointer<tkf::IOpenApi> pOpenApi = tkf::createOpenApi( apiConfig, authConfig, loggingConfig );

    tkf::ISanboxOpenApi* pSandboxOpenApi = tkf::getSandboxApi(pOpenApi);

    if (pSandboxOpenApi)
    {
        //------------------------------
        auto sandboxRegisterRes = pSandboxOpenApi->sandboxRegister(tkf::BrokerAccountType::eBrokerAccountType::TINKOFF); // TINKOFFIIS
        sandboxRegisterRes->join();
        tkf::checkAbort(sandboxRegisterRes);

        pSandboxOpenApi->setBrokerAccountId( sandboxRegisterRes->value.getPayload().getBrokerAccountId() );
    }


    //----------------------------------------------------------------------------

    QVector< QSharedPointer< tkf::OpenApiCompletableFuture< tkf::MarketInstrumentListResponse > > > instrumentResults;
    instrumentResults.push_back( pOpenApi->marketInstruments( tkf::InstrumentType("STOCK"   ) ) );
    instrumentResults.push_back( pOpenApi->marketInstruments( tkf::InstrumentType("CURRENCY") ) );
    instrumentResults.push_back( pOpenApi->marketInstruments( tkf::InstrumentType("BOND"    ) ) );
    instrumentResults.push_back( pOpenApi->marketInstruments( tkf::InstrumentType("ETF"     ) ) );

    tkf::joinOpenApiCompletableFutures(instrumentResults);

    QList<tkf::MarketInstrument> allInstruments;

    for( auto rsp : instrumentResults )
    {
        auto marketInstrumentList = rsp->value.getPayload();
        auto instrumentsList      = marketInstrumentList.getInstruments();

        for( auto instrumentInfo : instrumentsList )
        {
            /* 
               !!!

               Чего делаем:
                 Пробуем найти инструмент
                 Если его не существует в базе - добавляем
                 Если он существует в базе - только обновляем
                 Если что-то ошло не так - пропускаем инструмент

               Есть два вида заявок (обычно; вообще есть много вариантов, но у нас только так)
               Лимитная и рыночная заявки:
                 Лимитная - продать/купить по цене не ниже/выше заданной
                 Рыночная - купить что продают, по той цене, какую просят; ну или продать по той цене, какую дают
               Рыночные заявки исполняются обычно сразу

               В таблице инструментов поле LOT - это размер лота, который отдаёт тиньков API.
               Но тут есть нюанс.

               Если взять инструмент "доллары" (USD), то они торгуются по лимитным заявкам лотами по $1000
               По рыночным заявкам они торгуются по одному $1

               В основном размеры лотов (как минимум по акциям/STOCK) одни и те же для лимитных и для рыночных заявок.
               Но вот для USD обнаружилась разница.

               Поэтому было инжектировано поле LOT_MARKET, которое задаёт размер лота для рыночной заявки.

               При создании (первичном добавлении инструмента в базу) LOT_MARKET копируется из LOT.

               Потом можно ручками (или SQL-запросом) задать для LOT_MARKET другое значение
            
             */

            // LOT_MARKET

            //inline QVector<QString> removeFirstItems(QVector<QString> v, int numItemsToRemove )
            //inline QVector<QString> removeItemsByName( const QVector<QString> &v, const QString &name )

            qDebug().nospace().noquote() << tkf::modelToStrings( instrumentInfo );


        }

    }

    //getInstruments()
    //QVector<tkf::MarketInstrumentList>



    /*
    for( auto response : instrumentResults )
    {
        auto marketInstrumentList = response->value.getPayload();
        auto instrumentsList = marketInstrumentList.getInstruments();

        for(const auto &instrument : instrumentsList)
        {
        
        }
    }
    */
    /*
    for(const auto &instrument : list)
    {
        auto key = instrument.isin.toUpper();
        if (key.isEmpty())
            continue;
        resMap[key] = instrument.figi;
    }

     */

    if (pSandboxOpenApi)
    {
        auto 
        res = pSandboxOpenApi->sandboxClear();
        res->join();
        tkf::checkAbort(res);

        res = pSandboxOpenApi->sandboxRemove();
        res->join();
        tkf::checkAbort(res);
    }



    return 0;
}



