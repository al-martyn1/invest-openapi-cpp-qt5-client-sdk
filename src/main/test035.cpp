/*! \file
    \brief Тестируем статистику на стакане, получаемом по Streaming API

    Streaming API    - https://tinkoffcreditsystems.github.io/invest-openapi/marketdata/

    Свечи
      candle:subscribe
      candle:unsubscribe

    Стакан
      orderbook:subscribe
      orderbook:unsubscribe

    Инфа по инструменту
      тут из нужного - статус - торгуется или нет - "trade_status": "normal_trading" / "not_available_for_trading"
      Дельта цены и размер лота
      "min_price_increment": 0.0025,
      "lot": 1000
      
      instrument_info:subscribe
      instrument_info:unsubscribe


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




INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("test035");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;

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


    QStringList instrumentList;
    {
        QSettings settings(test032FullFileName, QSettings::IniFormat);
        instrumentList = settings.value("instruments" ).toStringList();
    }

    

    //QStringList   figis;

    tkf::DatabaseDictionaries dicts = tkf::DatabaseDictionaries(pMainDbMan);


    cout<<"#" << endl;

    QWebSocket webSocket;
    console_helpers::SimpleHandleCtrlC ctrlC; // ctrlC.isBreaked()


    std::atomic<bool> fConnected = false;


    auto onConnected = [&]()
            {
                using std::cout;  using std::endl;
            
                fConnected.store( true, std::memory_order_seq_cst  );

                cout << "# Streaming API Web socket connected" << endl;

                for( const auto instrument : instrumentList )
                {

                    QString figi = dicts.findFigiByAnyIdString(instrument);

                    if (figi.isEmpty())
                    {
                        std::set<QString> foundFigis = dicts.findFigisByName( instrument ); 
                        if ( foundFigis.empty() )
                        {
                            throw std::runtime_error( std::string("Instrument ") + instrument.toStdString() + std::string(" - not found all") );
                        }
                        if ( foundFigis.size()>1 )
                        {
                            throw std::runtime_error( std::string("Instrument ") + instrument.toStdString() + std::string(" - multiple variants was found") );
                        }
                    }

                    QString ticker = dicts.getTickerByFigiChecked(figi);

                    QString orderBookSubscriptionText      = pOpenApi->getStreamingApiOrderbookSubscribeJson( figi );
                    QString instrumentInfoSubscriptionText = pOpenApi->getStreamingApiInstrumentInfoSubscribeJson( figi );

                    cout << "# Subscribe to orderbook for " << ticker << " (" << figi << ")" << endl;
                    QTest::qWait(5);
                    webSocket.sendTextMessage( orderBookSubscriptionText );

                    cout << "# Subscribe to instrument info for " << ticker << " (" << figi << ")" << endl;
                    QTest::qWait(5);
                    webSocket.sendTextMessage( instrumentInfoSubscriptionText );

                }


            };

    auto onDisconnected = [&]()
            {
                using std::cout;  using std::endl;
            
                fConnected.store( false, std::memory_order_seq_cst  );

                cout << "# Streaming API Web socket disconnected" << endl;

                // Try to reconnect

                webSocket.open( pOpenApi->getStreamingApiNetworkRequest() );

            };

    auto onMessage = [&]( QString msg )
            {
                using std::cout;  using std::endl;

                tkf::GenericStreamingResponse genericStreamingResponse;
                genericStreamingResponse.fromJson(msg);

                auto eventName = genericStreamingResponse.getEvent();

                if (eventName=="error")
                {
                    tkf::StreamingError streamingError;
                    streamingError.fromJson(msg);

                    cout << "# !!! Streaming error: " << streamingError.getPayload().getMessage() << endl;
                }

                else if (eventName=="orderbook")
                {
                    tkf::StreamingOrderbookResponse orderbookResponse;
                    orderbookResponse.fromJson(msg);

                    tkf::MarketGlass marketGlass = tkf::MarketGlass::fromStreamingOrderbookResponse(orderbookResponse);
                    cout << "#---------------------" << endl;
                    cout << marketGlass << endl << endl ;
                    cout << marketGlass.getGlassMaxPrice();
                    cout << marketGlass.getGlassMinPrice();
                    cout << marketGlass << endl << endl ;
                    
                }


                    // StreamingCandleResponse

                    //   QString getTimeAsString()
                    //   bool is_time_Set() const;
                    //   bool is_time_Valid() const;

                    //   Candle getPayload()
                    //   bool is_payload_Set() const;
                    //   bool is_payload_Valid() const;


                    // StreamingInstrumentInfoResponse

                    //   QString getTimeAsString() const;
                    //   bool is_time_Set() const;
                    //   bool is_time_Valid() const;
                   
                    //   StreamingInstrumentInfo getPayload() const;
                    //   bool is_payload_Set() const;
                    //   bool is_payload_Valid() const;


                    // StreamingInstrumentInfo

                    //   QString getFigi() const;
                    //   bool is_figi_Set() const;
                    //   bool is_figi_Valid() const;

                    //   TradeStatus getTradeStatus() const;
                    //   bool is_trade_status_Set() const;
                    //   bool is_trade_status_Valid() const;
                   
                    //   marty::Decimal getMinPriceIncrement() const;
                    //   bool is_min_price_increment_Set() const;
                    //   bool is_min_price_increment_Valid() const;
                   
                    //   qint32 getLot() const;
                    //   bool is_lot_Set() const;
                    //   bool is_lot_Valid() const;


                    // StreamingOrderbookResponse

                    //   QString getTimeAsString() const;
                    //   bool is_time_Set() const;
                    //   bool is_time_Valid() const;

                    //   StreamingOrderbook getPayload() const;
                    //   bool is_payload_Set() const;
                    //   bool is_payload_Valid() const;


                    // StreamingOrderbook

                    //   QString getFigi() const;
                    //   bool is_figi_Set() const;
                    //   bool is_figi_Valid() const;
                   
                    //   qint32 getDepth() const;
                    //   bool is_depth_Set() const;
                    //   bool is_depth_Valid() const;
                   
                    //   QList< QList<marty::Decimal> > getBids() const;
                    //   bool is_bids_Set() const;
                    //   bool is_bids_Valid() const;
                   
                    //   QList< QList<marty::Decimal> > getAsks() const;
                    //   bool is_asks_Set() const;
                    //   bool is_asks_Valid() const;


                    // StreamingOrderbookItem

                    //   marty::Decimal   price;
                    //   marty::Decimal   quantity;
                    //   static StreamingOrderbookItem fromList( const QList< marty::Decimal > &list )


            };


    webSocket.connect( &webSocket, &QWebSocket::connected             , onConnected    );
    webSocket.connect( &webSocket, &QWebSocket::disconnected          , onDisconnected );
    webSocket.connect( &webSocket, &QWebSocket::textMessageReceived   , onMessage      );

    webSocket.open( pOpenApi->getStreamingApiNetworkRequest() );


    cout << "# Press Ctrl+C to break process" << endl;

    cout<<"#" << endl;


    while(!ctrlC.isBreaked())
    {
        QTest::qWait(1);

    }

    if (fConnected.load()!=false)
    {

       cout << "# WebSocket forced closing" << endl;

       webSocket.close();

       while(fConnected.load()!=false)
       {
           QTest::qWait(1);
       }

       cout << "# WebSocket closed" << endl;

    }

/*
    connect(&m_webSocket, &QWebSocket::connected, this, &EchoClient::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &EchoClient::closed);

    connect(&m_webSocket, &QWebSocket::textMessageReceived,
            this, &EchoClient::onTextMessageReceived);
*/
    
    return 0;
}



