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


    auto dbConfigFullFileName    = lookupForConfigFile( "database.properties", lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto logConfigFullFileName   = lookupForConfigFile( "logging.properties" , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto apiConfigFullFileName   = lookupForConfigFile( "config.properties"  , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto authConfigFullFileName  = lookupForConfigFile( "auth.properties"    , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );

    qDebug().nospace().noquote() << "DB   Config File: "<< dbConfigFullFileName   ;
    qDebug().nospace().noquote() << "Log  Config File: "<< logConfigFullFileName  ;
    qDebug().nospace().noquote() << "API  Config File: "<< apiConfigFullFileName  ;
    qDebug().nospace().noquote() << "Auth Config File: "<< authConfigFullFileName ;


    QSharedPointer<tkf::DatabaseConfig> pDatabaseConfig = QSharedPointer<tkf::DatabaseConfig>( new tkf::DatabaseConfig(dbConfigFullFileName, tkf::DatabasePlacementStrategyDefault()) );
    QSharedPointer<tkf::LoggingConfig>  pLoggingConfig  = QSharedPointer<tkf::LoggingConfig> ( new tkf::LoggingConfig(logConfigFullFileName) );

    qDebug().nospace().noquote() << "Main DB name: " << pDatabaseConfig->dbMainFilename;

    QSharedPointer<QSqlDatabase> pMainSqlDb = QSharedPointer<QSqlDatabase>( new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE")) );
    pMainSqlDb->setDatabaseName( pDatabaseConfig->dbMainFilename );

    if (!pMainSqlDb->open())
    {
        qDebug() << pMainSqlDb->lastError().text();
        return 0;
    }

    QSharedPointer<tkf::IDatabaseManager> pMainDbMan = tkf::createMainDatabaseManager( pMainSqlDb, pDatabaseConfig, pLoggingConfig );
    pMainDbMan->applyDefDecimalFormatFromConfig( *pDatabaseConfig );


    auto apiConfig     = tkf::ApiConfig    ( apiConfigFullFileName  );
    auto authConfig    = tkf::AuthConfig   ( authConfigFullFileName );
    auto loggingConfig = tkf::LoggingConfig( logConfigFullFileName  );

    QSharedPointer<tkf::IOpenApi> pOpenApi = tkf::createOpenApi( apiConfig, authConfig, loggingConfig );

    tkf::ISanboxOpenApi* pSandboxOpenApi = tkf::getSandboxApi(pOpenApi);

    if (pSandboxOpenApi)
    {
        pSandboxOpenApi->setBrokerAccountId( authConfig.getBrokerAccountId() );
    }
    else
    {
        pOpenApi->setBrokerAccountId( authConfig.getBrokerAccountId() );
    }


    //----------------------------------------------------------------------------
    QElapsedTimer mainTimer;
    mainTimer.start();

    QVector< QSharedPointer< tkf::OpenApiCompletableFuture< tkf::MarketInstrumentListResponse > > > instrumentResults;
    instrumentResults.push_back( pOpenApi->marketInstruments( tkf::InstrumentType("STOCK"   ) ) );
    instrumentResults.push_back( pOpenApi->marketInstruments( tkf::InstrumentType("CURRENCY") ) );
    instrumentResults.push_back( pOpenApi->marketInstruments( tkf::InstrumentType("BOND"    ) ) );
    instrumentResults.push_back( pOpenApi->marketInstruments( tkf::InstrumentType("ETF"     ) ) );

    qDebug().nospace().noquote() << "TIMER: Quering instruments, time elapsed: " << mainTimer.restart();

    tkf::joinOpenApiCompletableFutures(instrumentResults);

    qDebug().nospace().noquote() << "TIMER: Waiting instrument responses, time elapsed: " << mainTimer.restart();


    QVector<QString> instrumentColsWithLotMarket = tkf::removeItemsByName( pMainDbMan->tableGetColumnsFromSchema("MARKET_INSTRUMENT"), "ID" );
    QVector<QString> instrumentColsNoLotMarket   = tkf::removeItemsByName(instrumentColsWithLotMarket, "LOT_MARKET");
    int lotFieldIdx                              = tkf::findItemByName(instrumentColsWithLotMarket, "LOT");


    QVector<QString> instrumentTypes = { "STOCK", "CURRENCY", "BOND", "ETF" };
    int instrumentTypeIdx = -1;

    //QList<tkf::MarketInstrument> allInstruments;

    unsigned instrumentCount = 0;

    for( auto rsp : instrumentResults )
    {
        ++instrumentTypeIdx;

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
                 Если что-то пошло не так - пропускаем инструмент

               Есть два вида заявок (обычно; вообще есть много вариантов, но у нас только так)
               Лимитная и рыночная заявки:
                 Лимитная - продать/купить по цене не ниже/выше заданной
                 Рыночная - купить что продают, по той цене, какую просят; ну или продать по той цене, какую дают
               Рыночные заявки исполняются обычно сразу

               В таблице инструментов поле LOT - это размер лота, который отдаёт тиньков API.
               Но тут есть нюанс.

               Если взять инструмент "доллары" (USD), то они торгуются по лимитным заявкам лотами по $1000
               По рыночным заявкам они торгуются по одному ($1)

               В основном размеры лотов (как минимум по акциям/STOCK) одни и те же для лимитных и для рыночных заявок.
               Но вот для USD обнаружилась разница.

               Поэтому было инжектировано поле LOT_MARKET, которое задаёт размер лота для рыночной заявки.

               При создании (первичном добавлении инструмента в базу) LOT_MARKET копируется из LOT.

               Потом можно ручками (или SQL-запросом) задать для LOT_MARKET другое значение

               При обновлении инструмента LOT_MARKET не изменяется
            
             */

            //inline QVector<QString> removeFirstItems(QVector<QString> v, int numItemsToRemove )
            //inline QVector<QString> removeItemsByName( const QVector<QString> &v, const QString &name )

            QElapsedTimer singleInstrumentTimer;
            singleInstrumentTimer.start();

            QVector<QString> values = tkf::modelToStrings( instrumentInfo );
            qDebug().nospace().noquote() << values;

            //QString instrumentFigi = instrumentInfo.getFigi();
            //QString selectQuery = pMainDbMan->makeSimpleSelectQueryText( "MARKET_INSTRUMENT", "FIGI", instrumentFigi, instrumentColsNoLotMarket );
            QString selectQuery = pMainDbMan->makeSimpleSelectQueryText( "MARKET_INSTRUMENT", "FIGI", instrumentInfo.getFigi(), instrumentColsNoLotMarket );
            
            std::size_t selectResSize = pMainDbMan->getQueryResultSize( pMainDbMan->execHelper(selectQuery) );
            if (selectResSize>0)
            {
                qDebug().nospace().noquote() << "Updating instrument, FIGI = " << instrumentInfo.getFigi();
                QString updateQuery = pMainDbMan->makeSimpleUpdateQueryText( "MARKET_INSTRUMENT", "FIGI", instrumentInfo.getFigi(), values, instrumentColsNoLotMarket );
                pMainDbMan->execHelper( updateQuery );
            }
            else
            {
                QString lotValue;
                if (lotFieldIdx<values.size())
                    lotValue = values[lotFieldIdx];
                values.insert(lotFieldIdx+1, lotValue); // diplicate LOT to LOT_MARKET

                qDebug().nospace().noquote() << "Creating instrument, FIGI = " << instrumentInfo.getFigi();

                pMainDbMan->insertTo( "MARKET_INSTRUMENT", values, instrumentColsWithLotMarket );
            }

            ++instrumentCount;

            qDebug().nospace().noquote() << "Create/update single instrument timeout: "<<singleInstrumentTimer.restart();

        }

    }

    pMainDbMan->execHelper( pMainDbMan->makeSimpleUpdateQueryText( "MARKET_INSTRUMENT", "TICKER", "USD000UTSTOM"
                                                         , QVector<QString>{ "1" }, QVector<QString>{ "LOT_MARKET" }
                                                         )
                      );

    pMainDbMan->execHelper( pMainDbMan->makeSimpleUpdateQueryText( "MARKET_INSTRUMENT", "TICKER", "EUR_RUB__TOM"
                                                         , QVector<QString>{ "1" }, QVector<QString>{ "LOT_MARKET" }
                                                         )
                      );

    qDebug().nospace().noquote() << "TIMER: Update instruments job full time elapsed: " << mainTimer.restart();
    qDebug().nospace().noquote() << "Total instruments processed: " << instrumentCount;

    return 0;
}



