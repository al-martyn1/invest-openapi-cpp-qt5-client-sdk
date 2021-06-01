/*! \file
    \brief Тестируем статистику на стакане (без Streaming API)
 */

#include <iostream>
#include <exception>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <optional>
#include <atomic>

#include <QCoreApplication>
#include <QString>
#include <QSettings>
#include <QTest>
#include <QDir>
#include <QElapsedTimer>
#include <QtWebSockets>

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

#include "invest_openapi/console_break_helper.h"


#include "invest_openapi/streaming_handlers.h"

#include "invest_openapi/market_glass.h"
#include "invest_openapi/market_instrument_state.h"




INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("test036");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;
    using marty::Decimal;

    cout<<"# Path to exe   : "<<QCoreApplication::applicationDirPath().toStdString()<<endl;

    cout << "#" << endl;

    namespace tkf=invest_openapi;
    using tkf::config_helpers::lookupForConfigFile;
    using tkf::config_helpers::FileReadable;

    QStringList lookupConfSubfolders = QString("conf;config").split( ';', Qt::SkipEmptyParts );

    auto logConfigFullFileName     = lookupForConfigFile( "logging.properties" , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto apiConfigFullFileName     = lookupForConfigFile( "config.properties"  , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto authConfigFullFileName    = lookupForConfigFile( "auth.properties"    , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto dbConfigFullFileName      = lookupForConfigFile( "database.properties", lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto test032FullFileName       = lookupForConfigFile( "test032.properties" , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );

    cout << "# Log  Config File: "<< logConfigFullFileName   << endl;
    cout << "# API  Config File: "<< apiConfigFullFileName   << endl;
    cout << "# Auth Config File: "<< authConfigFullFileName  << endl;
    cout << "# Test032 Cfg File: "<< test032FullFileName     << endl;

    auto apiConfig     = tkf::ApiConfig    ( apiConfigFullFileName  );
    auto authConfig    = tkf::AuthConfig   ( authConfigFullFileName );

    QSharedPointer<tkf::DatabaseConfig> pDatabaseConfig = QSharedPointer<tkf::DatabaseConfig>( new tkf::DatabaseConfig(dbConfigFullFileName, tkf::DatabasePlacementStrategyDefault()) );

    QSharedPointer<tkf::LoggingConfig>  pLoggingConfig  = QSharedPointer<tkf::LoggingConfig> ( new tkf::LoggingConfig(logConfigFullFileName) );

    pLoggingConfig->debugSqlQueries = false;

    auto loggingConfig = *pLoggingConfig;



    cout << "# Main DB name: " << pDatabaseConfig->dbMainFilename << endl;

    QSharedPointer<QSqlDatabase> pMainSqlDb = QSharedPointer<QSqlDatabase>( new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE")) );
    pMainSqlDb->setDatabaseName( pDatabaseConfig->dbMainFilename );

    if (!pMainSqlDb->open())
    {
      //qDebug() 
      cout << pMainSqlDb->lastError().text() << endl;
      return 0;
    }

    QSharedPointer<tkf::IDatabaseManager> pMainDbMan = tkf::createMainDatabaseManager( pMainSqlDb, pDatabaseConfig, pLoggingConfig );

    pMainDbMan->applyDefDecimalFormatFromConfig( *pDatabaseConfig );


    // MAIL
    // 0.2
    QString json = "{\"payload\":{\"figi\":\"BBG00178PGX3\",\"depth\":20,\"bids\":[[1731,69],[1730.8,11],[1730.6,38],[1730.4,151],[1730.2,21],[1730,602],[1729.8,240],[1729.6,17],[1729.2,22],[1729,12],[1728.8,1],[1728.6,2],[1728.4,101],[1728.2,20],[1728,26],[1727.8,75],[1727.6,5683],[1727.4,400],[1727.2,200],[1727,81]],\"asks\":[[1731.6,100],[1732,20],[1732.2,58],[1732.4,61],[1732.6,51],[1732.8,189],[1733,1508],[1733.2,111],[1733.4,26],[1733.6,210],[1733.8,31],[1734,59],[1734.4,20],[1734.6,200],[1735,212],[1735.4,230],[1735.6,200],[1735.8,655],[1736,1205],[1736.2,1]]},\"event\":\"orderbook\",\"time\":\"2021-04-23T18:58:44.557787641Z\"}";

    tkf::StreamingOrderbookResponse response;
    response.fromJson(json);

    tkf::MarketGlass marketGlass = tkf::MarketGlass::fromStreamingOrderbookResponse(response);
    Decimal priceIncrement = Decimal("0.2");


    
    cout << marketGlass << endl; // << endl ;
    cout << "Max price  :  " << marketGlass.getGlassMaxPrice() << endl;
    cout << "Min price  :  " << marketGlass.getGlassMinPrice() << endl;
    cout << endl;

    cout << "Asks range : " << marketGlass.getAsksMinPrice() << " - " << marketGlass.getAsksMaxPrice() << endl;
    cout << "Bids range : " << marketGlass.getBidsMinPrice() << " - " << marketGlass.getBidsMaxPrice() << endl;

    cout << endl;

    cout << "Best ask   :  " << marketGlass.getAskBestPrice() << endl;
    cout << "Best bid   :  " << marketGlass.getBidBestPrice() << endl;
    cout << "Spread     :  " << marketGlass.getPriceSpread() << endl;
    auto instrumentPrice = marketGlass.calcInstrumentPrice( priceIncrement );
    cout << "Cur price  :  " << instrumentPrice << endl;

    cout << endl;

    cout << "Total asks :  " << marketGlass.getQuantityAsks() << ", ratio to bids: " << marketGlass.getAsksBidsRatio() << endl;
    cout << "Total bids :  " << marketGlass.getQuantityBids() << ", ratio to asks: " << marketGlass.getBidsAsksRatio() << endl;
    
    cout << endl;

    cout << "Asks HQ Ratio: " << marketGlass.calcAsksHighQuantityRatio( marketGlass.getAsksQuantityOutlierLimits() ) << endl;
    cout << "Bids HQ Ratio: " << marketGlass.calcBidsHighQuantityRatio( marketGlass.getBidsQuantityOutlierLimits() ) << endl;

    cout << endl;

    /*
    bool instrumentInfoFound = marketInstrumentsState.find(marketGlass.figi)!=marketInstrumentsState.end();
    cout << "Instrument Info: " << ( instrumentInfoFound? "not " : "" ) << "found" << (!instrumentInfoFound ? " ////???" : "") << endl;
    */

    // Here is a lot of advanced info

    cout << endl;
    cout << "Price increment: " << priceIncrement << endl;
    cout << "Spread points  : " << marketGlass.getPriceSpreadPoints( priceIncrement ) << endl;

    cout << endl;
    cout << endl;

    tkf::MarketGlass sparsedGlass = marketGlass.getSparsed( priceIncrement );

    cout << sparsedGlass << endl; // << endl ;

    cout << endl;

    std::size_t asksSparsedSize = marketGlass.getAsksSparsedSize(priceIncrement);
    std::size_t bidsSparsedSize = marketGlass.getBidsSparsedSize(priceIncrement);

    Decimal asksSparseScale = marketGlass.getAsksSparseScale(priceIncrement);
    Decimal bidsSparseScale = marketGlass.getBidsSparseScale(priceIncrement);

    Decimal asksSparsePercent = marketGlass.getAsksSparsePercent(priceIncrement);
    Decimal bidsSparsePercent = marketGlass.getBidsSparsePercent(priceIncrement);

    cout << "Asks sparse size: " << asksSparsedSize << ", scale: " << asksSparseScale << ", " << asksSparsePercent << "%" << endl;
    cout << "Bids sparse size: " << bidsSparsedSize << ", scale: " << bidsSparseScale << ", " << bidsSparsePercent << "%" << endl;

    cout << endl;
    cout << endl;

    
    return 0;
}



