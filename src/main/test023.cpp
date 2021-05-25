/*! \file
    \brief Configs lookup test

 */

#include <iostream>
#include <exception>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <map>
#include <set>

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
#include "invest_openapi/balance_config.h"

#include "invest_openapi/invest_openapi.h"
#include "invest_openapi/factory.h"
#include "invest_openapi/openapi_completable_future.h"

#include "invest_openapi/database_config.h"
#include "invest_openapi/database_manager.h"
#include "invest_openapi/qt_time_helpers.h"

#include "invest_openapi/db_utils.h"
#include "invest_openapi/ioa_utils.h"
#include "invest_openapi/ioa_ostream.h"
#include "invest_openapi/ioa_db_dictionaries.h"
#include "cpp/cpp.h"



INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("test023");
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


    auto logConfigFullFileName     = lookupForConfigFile( "logging.properties" , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto apiConfigFullFileName     = lookupForConfigFile( "config.properties"  , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto authConfigFullFileName    = lookupForConfigFile( "auth.properties"    , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto dbConfigFullFileName      = lookupForConfigFile( "database.properties", lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    //auto balanceConfigFullFileName = lookupForConfigFile( "balance.properties" , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );

    qDebug().nospace().noquote() << "Log  Config File: "<< logConfigFullFileName  ;
    qDebug().nospace().noquote() << "API  Config File: "<< apiConfigFullFileName  ;
    qDebug().nospace().noquote() << "Auth Config File: "<< authConfigFullFileName ;
    qDebug().nospace().noquote() << "DB   Config     : "<< dbConfigFullFileName   ;
    //qDebug().nospace().noquote() << "Balance Config  : "<< balanceConfigFullFileName;

    auto apiConfig     = tkf::ApiConfig    ( apiConfigFullFileName  );
    auto authConfig    = tkf::AuthConfig   ( authConfigFullFileName );
    //auto balanceConfig = tkf::BalanceConfig( balanceConfigFullFileName );
    //auto balance_config.h


    QSharedPointer<tkf::DatabaseConfig> pDatabaseConfig = QSharedPointer<tkf::DatabaseConfig>( new tkf::DatabaseConfig(dbConfigFullFileName, tkf::DatabasePlacementStrategyDefault()) );
    QSharedPointer<tkf::LoggingConfig>  pLoggingConfig  = QSharedPointer<tkf::LoggingConfig> ( new tkf::LoggingConfig(logConfigFullFileName) );
    auto loggingConfig = *pLoggingConfig;


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


    tkf::DatabaseDictionaries dicts = tkf::DatabaseDictionaries(pMainDbMan);

    // Нормально работает
    dicts.dictLoadTest( pMainDbMan, "MARKET_INSTRUMENT", "FIGI,ID" );

    // Кидает исключение из кишочек DbMan'а - такой таблицы по просту нет
    //dicts.dictLoadTest( pMainDbMan, "HPEN", "FIGI,ID" );

    // Кидает исключение при загрузке словаря (ну, если в базу данные по свечам ещё не были залиты) - тестировал отработку пустого словаря, а базу пересоздавать было лень
    //dicts.dictLoadTest( pMainDbMan, "INSTRUMENT_CANDLES", "CANDLE_DATE_TIME,INSTRUMENT_ID" );


    // std::set<QString> findFigisByName( QString name ) const
    // QString getTickerByFigi( QString &figi ) const
    // QString findFigiByAnyIdStringChecked( QString idStr ) const
    // QString findFigiByAnyIdString( QString idStr ) const
    // QString getNameByFigi( QString figi ) const


    // BBG004731354 - ROSN

    QString testTicker = "ROSN";

    cout << "Test TICKER: " << testTicker << endl;

    QString testFigi         = dicts.findFigiByAnyIdStringChecked(testTicker);
    QString testTickerByFigi = dicts.getTickerByFigiChecked(testFigi);

    cout << "Test FIGI  : " << testFigi << ", TICKER: " << testTickerByFigi;
    QString name = dicts.getNameByFigiChecked(testFigi);
    if (!name.isEmpty())
        cout << " - " << name;
    cout << endl;

    int testId = dicts.getInstrumentIdChecked(testFigi);
    cout << "Instrument Id   : " << testId << endl;
    cout << "Instrument by Id: " << dicts.getInstrumentByIdChecked(testId) << endl;

    cout << endl;

    // Need FIGI -> ISIN methods for complete API?

    //QString substringToFind = "energy"; // Found, case ignored
    QString substringToFind = "nergy"; // try here to find the only part of name

    cout << "Lookup for FIGIs by '" << substringToFind << "' substring in name" << endl;

    std::set<QString> energyFigis = dicts.findFigisByName( substringToFind ); 

    for( auto foundFigi : energyFigis )
    {
        cout << "    " << foundFigi << " : " << cpp::expandAtBack(dicts.getNameByFigiChecked(foundFigi).toStdString(), 32) 
             << ", TICKER: " << cpp::expandAtBack(dicts.getTickerByFigiChecked(foundFigi).toStdString(), 6)
             << ", ID: " << dicts.getInstrumentIdChecked(foundFigi) 
             << endl;
    }

    cout << endl;

    cout << "Dictionary tests" << endl;

    #define DICTIONARY_SIMPLE_TEST( testTitle, testValue, testMethod )                                                                    \
                do                                                                                                                        \
                {                                                                                                                         \
                    QString testStr     = testValue;                                                                                      \
                    int     id          = dicts.get ##testMethod ##IdChecked( testStr );                                                  \
                    QString nameNyId    = dicts.get ##testMethod ##ByIdChecked( id );                                                     \
                    cout << testTitle << ": " << cpp::expandAtBack(testStr.toStdString(), 10) << ", ID: " << id << ", String ID: " << nameNyId << endl; \
                } while(0)

    // DICTIONARY_SIMPLE_TEST( "BrokerAccountType test, Acc Type", "usd", BrokerAccountType ); // throws an exception
    DICTIONARY_SIMPLE_TEST( "BrokerAccountType test           , Acc Type                   ", "Tinkoff"        , BrokerAccountType               );

    DICTIONARY_SIMPLE_TEST( "CandleResolution test            , Resolution                 ", "5min"           , CandleResolution                );
    DICTIONARY_SIMPLE_TEST( "CandleResolutionIntervalMin test , Interval Min               ", "15m"            , CandleResolutionIntervalMin     ); // Тут нормально работает только get*ById, потому что строки не уникальны
    DICTIONARY_SIMPLE_TEST( "CandleResolutionIntervalMax test , Interval Max               ", "7d"             , CandleResolutionIntervalMax     ); // Тут нормально работает только get*ById, потому что строки не уникальны

    DICTIONARY_SIMPLE_TEST( "Currency test                    , Currency                   ", "usd"            , Currency                        );

    DICTIONARY_SIMPLE_TEST( "InstrumentType test              , InstrumentType             ", "bond"           , InstrumentType                  );

    DICTIONARY_SIMPLE_TEST( "OperationStatus test             , OperationStatus            ", "done"           , OperationStatus                 );

    DICTIONARY_SIMPLE_TEST( "OperationType test               , OperationType              ", "sell"           , OperationType                   );

    DICTIONARY_SIMPLE_TEST( "OperationTypeWithCommission test , OperationTypeWithCommission", "othercommission", OperationTypeWithCommission     );

    DICTIONARY_SIMPLE_TEST( "OrderStatus test                 , OrderStatus                ", "cancelled"      , OrderStatus                     );

    DICTIONARY_SIMPLE_TEST( "OrderType test                   , OrderType                  ", "market"         , OrderType                       );

    DICTIONARY_SIMPLE_TEST( "StockExangeList test             , StockExangeList            ", "moex"           , StockExangeList                 );

    



    /*            
    QString currencyName     = "usd";
    int currencyId           = dicts.getCurrencyId( currencyName );
    QString currencyNameNyId = dicts.getCurrencyById( currencyId );

    cout << "Currency test, currency: " << currencyName << ", ID: " << currencyId << ", NAME: " << currencyNameNyId << endl;
    */

    return 0;
}



