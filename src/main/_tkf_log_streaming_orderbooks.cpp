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
#include <fstream>
#include <exception>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <optional>
#include <atomic>
#include <memory>

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
    QCoreApplication::setApplicationName("_tkf_log_streaming_orderbooks");
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
    auto instrumentsConfigFullFileName = lookupForConfigFile( "instruments.properties" , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );

    cout << "# Log  Config File: "<< logConfigFullFileName   << endl;
    cout << "# API  Config File: "<< apiConfigFullFileName   << endl;
    cout << "# Auth Config File: "<< authConfigFullFileName  << endl;
    cout << "# Instruments Cfg File: "<< instrumentsConfigFullFileName << endl;

    auto apiConfig     = tkf::ApiConfig    ( apiConfigFullFileName  );
    auto authConfig    = tkf::AuthConfig   ( authConfigFullFileName );

    QSharedPointer<tkf::DatabaseConfig> pDatabaseConfig = QSharedPointer<tkf::DatabaseConfig>( new tkf::DatabaseConfig(dbConfigFullFileName, tkf::DatabasePlacementStrategyDefault()) );

    QSharedPointer<tkf::LoggingConfig>  pLoggingConfig  = QSharedPointer<tkf::LoggingConfig> ( new tkf::LoggingConfig(logConfigFullFileName) );

    pLoggingConfig->debugSqlQueries = false;

    auto loggingConfig = *pLoggingConfig;



    //auto dataLogFullFilename = pLoggingConfig->getDataLogFullName( logConfigFullFileName, "", "test.dat" );

    //cout << "# test_streaming_api data log file: "<< dataLogFullFilename << endl;




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
        QSettings settings(instrumentsConfigFullFileName, QSettings::IniFormat);
        instrumentList = settings.value("instruments" ).toStringList();
    }

    

    //QStringList   figis;

    tkf::DatabaseDictionaries dicts = tkf::DatabaseDictionaries(pMainDbMan);

    std::map< QString, tkf::MarketInstrumentState >        marketInstrumentsState;

    std::map< QString, std::shared_ptr< std::ofstream > >  marketInstrumentLogs;
    std::map< QString, std::shared_ptr< std::ofstream > >  marketInstrumentCsvs;

    std::map< QString, std::uint64_t >  timeStamps;



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

                    QString orderBookSubscriptionText         = pOpenApi->getStreamingApiOrderbookSubscribeJson( figi );
                    QString instrumentInfoSubscriptionText    = pOpenApi->getStreamingApiInstrumentInfoSubscribeJson( figi );
                    QString instrumentCandlesSubscriptionText = pOpenApi->getStreamingApiCandleSubscribeJson( figi, "1MIN" );

                    QTest::qWait(5);
                    cout << "# Subscribe to instrument info for " << ticker << " (" << figi << ")" << endl;
                    webSocket.sendTextMessage( instrumentInfoSubscriptionText );

                    QTest::qWait(5);
                    cout << "# Subscribe to orderbook for " << ticker << " (" << figi << ")" << endl;
                    webSocket.sendTextMessage( orderBookSubscriptionText );

                    QTest::qWait(5);
                    cout << "# Subscribe to instrument 1 MIN candles for " << ticker << " (" << figi << ")" << endl;
                    webSocket.sendTextMessage( instrumentCandlesSubscriptionText );


                    //std::map< QString, std::shared_ptr< std::ofstream > >::

                    if (marketInstrumentLogs.find(figi)==marketInstrumentLogs.end())
                    {
                        // Create logging and CSV streams

                        //pLoggingConfig->getDataLogFullName( logConfigFullFileName, "", ticker + QString("_") + figi + QString(".csv") );
                        QString logName = ticker + QString("_") + figi + QString(".txt");
                        QString csvName = ticker + QString("_") + figi + QString(".csv");

                        QString logFullName = pLoggingConfig->getDataLogFullName( logConfigFullFileName, "", logName );
                        QString csvFullName = pLoggingConfig->getDataLogFullName( logConfigFullFileName, "", csvName );

                        marketInstrumentLogs[figi] = std::make_shared<std::ofstream>( logFullName.toStdWString() );
                        marketInstrumentCsvs[figi] = std::make_shared<std::ofstream>( csvFullName.toStdWString() );
                    }
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

                tkf::GenericStreamingResponse response;
                response.fromJson(msg);

                auto eventName = response.getEvent();

                if (eventName=="error")
                {
                    tkf::StreamingError streamingError;
                    streamingError.fromJson(msg);

                    cout << "# !!! Streaming error: " << streamingError.getPayload().getMessage() << endl;
                }

                else if (eventName=="orderbook")
                {
                    tkf::StreamingOrderbookResponse response;
                    response.fromJson(msg);

                    tkf::MarketGlass marketGlass = tkf::MarketGlass::fromStreamingOrderbookResponse(response);

                    if (!marketGlass.isValid())
                    {
                        cout << "# !!! Streaming error: got an invalid MarketGlass - " << marketGlass.whyInvalid() << endl;
                        cout << "#     Input is: " << msg << endl;
                        
                    }
                    else
                    {
                        auto miStateIt = marketInstrumentsState.find(marketGlass.figi);
                        if (miStateIt!=marketInstrumentsState.end())
                        {
                            auto priceIncrement = miStateIt->second.priceIncrement;

                            auto ticker  = dicts.getTickerByFigiChecked(marketGlass.figi);
                            auto stampIt = timeStamps.find(marketGlass.figi);
                            std::uint64_t timeDelta = 0;
                            bool firstGlassData = true;

                            if (stampIt!=timeStamps.end() && (marketGlass.dateTimeAsStamp > stampIt->second) )
                            {
                                timeDelta = marketGlass.dateTimeAsStamp - stampIt->second;
                                firstGlassData = false;
                            }

                            auto asksLimits      = marketGlass.getAsksQuantityOutlierLimits();
                            auto bidsLimits      = marketGlass.getBidsQuantityOutlierLimits();

                            //------------------------------

                            auto glassMaxPrice                = marketGlass.getGlassMaxPrice();
                            auto glassMinPrice                = marketGlass.getGlassMinPrice();
                            auto glassPriceRange              = marketGlass.calcGlassPriceRange();
                            int  glassPriceRangePoints        = marketGlass.calcGlassPriceRangePoints( priceIncrement ); // int
                            auto instrumentPrice              = marketGlass.calcInstrumentPrice( priceIncrement );

                            auto glassBestAsk                 = marketGlass.getAskBestPrice();
                            auto glassBestBid                 = marketGlass.getBidBestPrice();
                            auto glassSpread                  = marketGlass.getPriceSpread();
                            int  glassSpreadPoints            = marketGlass.getPriceSpreadPoints(priceIncrement); // int

                            auto glassAsksPriceRange          = marketGlass.calcAsksPriceRange();
                            int  glassAsksPriceRangePoints    = marketGlass.calcAsksPriceRangePoints(priceIncrement);
                            auto glassBidsPriceRange          = marketGlass.calcBidsPriceRange();
                            int  glassBidsPriceRangePoints    = marketGlass.calcBidsPriceRangePoints(priceIncrement);

                            int  glassAsksQuantity            = marketGlass.getQuantityAsks();
                            auto glassAsksQuantityRatioToBids = marketGlass.getAsksBidsRatio();
                            auto glassAsksSparseScale         = marketGlass.getAsksSparseScale(priceIncrement);
                            auto glassAsksSparsePercent       = marketGlass.getAsksSparsePercent(priceIncrement);

                            int  glassBidsQuantity            = marketGlass.getQuantityBids();
                            auto glassBidsQuantityRatioToAsks = marketGlass.getBidsAsksRatio();
                            auto glassBidsSparseScale         = marketGlass.getBidsSparseScale(priceIncrement);
                            auto glassBidsSparsePercent       = marketGlass.getBidsSparsePercent(priceIncrement);

                            auto asksHqRatio                  = marketGlass.calcAsksHighQuantityRatio( asksLimits );
                            auto bidsHqRatio                  = marketGlass.calcBidsHighQuantityRatio( bidsLimits );

                            std::size_t asksOutlierDistance   = marketGlass.calcAsksDistanceToOutlier( asksLimits, priceIncrement );
                            std::size_t bidsOutlierDistance   = marketGlass.calcBidsDistanceToOutlier( bidsLimits, priceIncrement );
                            
                            auto logIt = marketInstrumentLogs.find(marketGlass.figi);
                            if (logIt!=marketInstrumentLogs.end())
                            {
                                std::ostream &log = * logIt->second;

                                log << "#---------------------" << endl;

                                log << endl;

                                log << "Ticker                      :  " << ticker << endl;
                                log << "Figi                        :  " << marketGlass.figi << endl;
                                log << "DateTime                    :  " << marketGlass.dateTimeString << endl;
                                log << "Time Stamp                  :  " << marketGlass.dateTimeAsStamp << endl;
                                log << "Stamp Delta                 :  " << timeDelta << endl;

                                log << endl;

                                log << "Max Price                   :  " << glassMaxPrice         << endl;
                                log << "Min Price                   :  " << glassMinPrice         << endl;
                                log << "Price Range                 :  " << glassPriceRange       << endl;
                                log << "Range Points                :  " << glassPriceRangePoints << endl; // int
                                log << "Cur Price                   :  " << instrumentPrice       << endl;

                                log << endl;

                                log << "Best Ask                    :  " << glassBestAsk      << endl;
                                log << "Best Bid                    :  " << glassBestBid      << endl;
                                log << "Spread                      :  " << glassSpread       << endl;
                                log << "Spread Points               :  " << glassSpreadPoints << endl;

                                log << endl;

                                log << "Asks Range                  :  " << glassAsksPriceRange       << endl;
                                log << "Asks Range Points           :  " << glassAsksPriceRangePoints << endl;
                                log << "Bids Range                  :  " << glassBidsPriceRange       << endl;
                                log << "Bids Range Points           :  " << glassBidsPriceRangePoints << endl;

                                log << endl;

                                log << "Asks Quantity               :  " << glassAsksQuantity            << endl;
                                log << "Asks Quantity to Bids Ratio :  " << glassAsksQuantityRatioToBids << endl;
                                log << "Asks Sparce Scale           :  " << glassAsksSparseScale         << endl;
                                log << "Asks Sparce Percent         :  " << glassAsksSparsePercent       << endl;

                                log << endl;

                                log << "Bids Quantity               :  " << glassBidsQuantity            << endl;
                                log << "Bids Quantity to Asks Ratio :  " << glassBidsQuantityRatioToAsks << endl;
                                log << "Bids Sparce Scale           :  " << glassBidsSparseScale         << endl;
                                log << "Bids Sparce Percent         :  " << glassBidsSparsePercent       << endl;

                                log << endl;

                                log << "Asks High Quantity Ratio    :  " << asksHqRatio << endl;
                                log << "Bids High Quantity Ratio    :  " << bidsHqRatio << endl;

                                log << endl;

                                log << "Glass Depth                 :  " << marketGlass.getGlassDepth() << endl;
                                log << "Asks Outlier Distance       :  " << asksOutlierDistance << endl;
                                log << "Bids Outlier Distance       :  " << bidsOutlierDistance << endl;

                                log << endl;

                                log << "#---" << endl;

                                log << endl;

                                log << marketGlass.getSparsed(priceIncrement) << endl;
                                log << endl;
                                log << endl;

                            }

                            auto csvIt = marketInstrumentCsvs.find(marketGlass.figi);
                            if (csvIt!=marketInstrumentCsvs.end())
                            {
                                std::ostream &csv = * csvIt->second;

                                char oldSep = marty::Decimal::getDefaultDecimalSeparator( );
                                marty::Decimal::setDefaultDecimalSeparator( ',' );

                                if (firstGlassData)
                                {
                                    csv << "Ticker;Figi;DateTime;Time Stamp;Stamp Delta;"
                                        << "Max Price;Min Price;Price Range;Range Points;Cur Price;"
                                        << "Best Ask;Best Bid;Spread;Spread Points;"
                                        << "Asks Range;Asks Range Points;Bid Range;Bid Range Points;"
                                        << "Asks Quantity;Asks Quantity to Bids Ratio;Asks Sparce Scale;Asks Sparce Percent;"
                                        << "Bids Quantity;Bids Quantity to Asks Ratio;Bids Sparce Scale;Bids Sparce Percent;"
                                        << "Asks High Quantity Ratio;Bids High Quantity Ratio;"
                                        << "Glass Depth;Asks Outlier Distance;Bids Outlier Distance"
                                        << endl;
                                }

                                csv << ticker << ";" << marketGlass.figi << ";" << marketGlass.dateTimeString << ";" << marketGlass.dateTimeAsStamp << ";" << timeDelta << ";"
                                    << glassMaxPrice << ";" << glassMinPrice << ";" << glassPriceRange << ";" << glassPriceRangePoints << ";" << instrumentPrice << ";"
                                    << glassBestAsk << ";" << glassBestBid << ";" << glassSpread << ";" << glassSpreadPoints << ";"
                                    << glassAsksPriceRange << ";" << glassAsksPriceRangePoints << ";" << glassBidsPriceRange << ";" << glassBidsPriceRangePoints << ";"
                                    << glassAsksQuantity << ";" << glassAsksQuantityRatioToBids << ";" << glassAsksSparseScale << ";" << glassAsksSparsePercent << ";"
                                    << glassBidsQuantity << ";" << glassBidsQuantityRatioToAsks << ";" << glassBidsSparseScale << ";" << glassBidsSparsePercent << ";"
                                    << asksHqRatio << ";" << bidsHqRatio << ";"
                                    << marketGlass.getGlassDepth() << ";" << asksOutlierDistance << ";" << bidsOutlierDistance << ";"
                                    << endl;

                                marty::Decimal::setDefaultDecimalSeparator( oldSep );
                            }


                        }

                        timeStamps[marketGlass.figi] = marketGlass.dateTimeAsStamp;

                        /*
                        cout << "#---------------------" << endl;
                        cout << marketGlass << endl; // << endl ;

                        cout << endl;

                        bool instrumentInfoFound = marketInstrumentsState.find(marketGlass.figi)!=marketInstrumentsState.end();
                        cout << "Instrument Info: " << ( !instrumentInfoFound ? "not " : "" ) << "found" << (!instrumentInfoFound ? " ////???" : "") << endl;

                        auto miStateIt = marketInstrumentsState.find(marketGlass.figi);
                        if (miStateIt!=marketInstrumentsState.end())
                        {
                            // Here is a lot of advanced info

                           
                            cout << endl;
                           
                            cout << "Asks Outlier distance: " << marketGlass.calcAsksDistanceToOutlier( asksLimits, priceIncrement ) << endl;
                            cout << "Bids Outlier distance: " << marketGlass.calcBidsDistanceToOutlier( bidsLimits, priceIncrement ) << endl;

                        }

                        cout << endl;
                        cout << endl;
                        //cout << marketGlass << endl << endl ;
                        */
                    }
                    
                }

                else if (eventName=="instrument_info")
                {
                    tkf::StreamingInstrumentInfoResponse response;
                    response.fromJson(msg);

                    tkf::MarketInstrumentState instrumentState = tkf::MarketInstrumentState::fromStreamingInstrumentInfoResponse( response );

                    if (!instrumentState.isValid())
                    {
                        cout << "# !!! Streaming error: got an invalid MarketInstrumentState - " << msg << endl;
                    }
                    else
                    {
                        marketInstrumentsState[instrumentState.figi] = instrumentState;

                        cout << "#---------------------" << endl;
                        cout << instrumentState << endl; // << endl ;

                    }
                    
                }

                else if (eventName=="candle")
                {
                    cout << "# !!! Candle event not handled, data: " << endl << msg << endl << endl;
                }

                else
                {
                    cout << "# !!! Unknown event: " << eventName << endl;
                    cout << msg << endl;
                }

                // std::map< QString, tkf::MarketInstrumentState >  marketInstrumentState;

                // https://bcs-express.ru/novosti-i-analitika/o-chem-mogut-rasskazat-birzhevoi-stakan-i-lenta-sdelok


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



