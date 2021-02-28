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


    qDebug().nospace().noquote() << "DB name: " << pDatabaseConfig->dbFilename;

    QSharedPointer<QSqlDatabase> pSqlDb = QSharedPointer<QSqlDatabase>( new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE")) );
    pSqlDb->setDatabaseName( pDatabaseConfig->dbFilename );

    if (!pSqlDb->open())
    {
      qDebug() << pSqlDb->lastError().text();
      return 0;
    }

    QSharedPointer<tkf::IDatabaseManager> pDbMan = tkf::createDatabaseManager( pSqlDb, pDatabaseConfig, pLoggingConfig );

    pDbMan->applyDefDecimalFormatFromConfig( *pDatabaseConfig );



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

    /*
    std::map< QString, int > figiToId              = pDbMan->getDictionaryFromTable  ( "MARKET_INSTRUMENT", "FIGI,ID"  );
    std::map< QString, int > tickerToId            = pDbMan->getDictionaryFromTable  ( "MARKET_INSTRUMENT", "TICKER,ID");
    std::map< QString, int > isinToId              = pDbMan->getDictionaryFromTable  ( "MARKET_INSTRUMENT", "ISIN,ID"  );
    std::map< int, QString > idToName              = pDbMan->getIdToFieldMapFromTable( "MARKET_INSTRUMENT", "ID,NAME"  );

    std::map< QString, int > stockExToId           = pDbMan->getDictionaryFromTable  ( "STOCK_EXCHANGE_LIST", "NAME,ID"  );

    std::map< QString, int > currencyToId          = pDbMan->getDictionaryFromTable  ( "CURRENCY", "NAME,ID"  );

    std::map< QString, int > instrumentTypeToId    = pDbMan->getDictionaryFromTable  ( "INSTRUMENT_TYPE", "TYPE,ID"  );

    std::map< QString, int > candleResolutionToId  = pDbMan->getDictionaryFromTable  ( "CANDLE_RESOLUTION", "RESOLUTION,ID"  );

    if ( figiToId.empty() || stockExToId.empty() || currencyToId.empty() )
    {
        qDebug().nospace().noquote() << "Something goes wrong. Possible DB is clean";
        return 1;
    }

    std::map< int, QString > idToFigi   = tkf::makeMapSwapKeyVal( figiToId );
    std::map< int, QString > idToTicker = tkf::makeMapSwapKeyVal( tickerToId );

    std::map< QString, QString > tickerToFigi = tkf::makeTransitionMap( tickerToId, idToFigi   );
    std::map< QString, QString > isinToFigi   = tkf::makeTransitionMap( isinToId  , idToFigi   );
    std::map< QString, QString > figiToTicker = tkf::makeTransitionMap( figiToId  , idToTicker );
    std::map< QString, QString > figiToName   = tkf::makeTransitionMap( figiToId  , idToName   );
    */

    tkf::DatabaseDictionaries dicts = tkf::DatabaseDictionaries(pDbMan);

    // Нормально работает
    dicts.dictLoadTest( pDbMan, "MARKET_INSTRUMENT", "FIGI,ID" );

    // Кидает исключение из кишочек DbMan'а - такой таблицы по просту нет
    //dicts.dictLoadTest( pDbMan, "HPEN", "FIGI,ID" );

    // Кидает исключение при загрузке словаря (ну, если в базу данные по свечам ещё не были залиты) - тестировал отработку пустого словаря, а базу пересоздавать было лень
    //dicts.dictLoadTest( pDbMan, "INSTRUMENT_CANDLES", "CANDLE_DATE_TIME,INSTRUMENT_ID" );


    // std::set<QString> findFigisByName( QString name ) const
    // QString getTickerByFigi( QString &figi ) const
    // QString findFigiByAnyIdSafe( QString idStr ) const
    // QString findFigiByAnyId( QString idStr ) const
    // QString getNameByFigi( QString figi ) const


    // BBG004731354 - ROSN

    QString testTicker = "ROSN";

    cout << "Test TICKER: " << testTicker << endl;

    QString testFigi = dicts.findFigiByAnyIdSafe(testTicker);

    cout << "Test FIGI  : " << testFigi;
    QString name = dicts.getNameByFigi(testFigi);
    if (!name.isEmpty())
        cout << " - " << name;
    cout << endl;

    // Need FIGI -> ISIN methods for complete API

    //QString substringToFind = "energy"; // Found, case ignored
    QString substringToFind = "nergy"; // try here to find the only part of name

    cout << "Lookup for FIGIs by '" << substringToFind << "' substring in name" << endl;

    std::set<QString> energyFigis = dicts.findFigisByName( substringToFind ); 

    for( auto foundFigi : energyFigis )
    {
        cout << "    " << foundFigi << " : " << dicts.getNameByFigi(foundFigi) << endl;
    }

    
    return 0;
}


