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


#include "invest_openapi/placed_order_info.h"

#include "invest_openapi/order_params.h"

#include "invest_openapi/cpp_helpers.h"





INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("_tkf_batch_place_orders");
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



    tkf::DatabaseDictionaries dicts = tkf::DatabaseDictionaries(pMainDbMan);



    std::string inputFileName;
    if (argc>1)
    {
        inputFileName = argv[1];
    }

    std::vector<std::string> inputLines;
    
    if (inputFileName.empty() || inputFileName=="-")
    {
        inputLines = tkf::cpp_helpers::readLines( std::cin );
    }
    else
    {
        inputLines = tkf::cpp_helpers::readLinesFromFile( inputFileName );
    }


    tkf::cpp_helpers::trimStrings( inputLines, false  /* !keepEmpty */  );


    std::vector< tkf::OrderParams > inputOrderParams;

    unsigned lineNo = 1;

    for( auto line : inputLines )
    {
        if (!line.empty() && line[0]!='#' && line[0]!=';')
        {
            std::vector< std::string > opArgs;
            tkf::cpp_helpers::splitToVector( line, opArgs, ' ' );
            tkf::cpp_helpers::trimStrings(opArgs);

            tkf::OrderParams orderParams;
            int orderParamsParsingRes = tkf::parseOrderParams( opArgs, dicts, orderParams );
            if (orderParamsParsingRes<0)
            {
                std::cerr << "Error: invalid parameter #" << -orderParamsParsingRes << " in line #" << lineNo << endl;
                return -orderParamsParsingRes;
            }

            if (!orderParams.isOrderTypeLimit())
            {
                std::cerr << "Error: in line #" << lineNo << ": Only limit orders supported" << endl;
                return 1;
            }

            inputOrderParams.push_back(orderParams);
        }
    
    }


    /*  Что делаем

        1) Раскладываем заявки по отдельным корзинам инструментов

        2) Коннектим веб сокет

        3) Делаем подписку на инструменты и стаканы по ним

        4) По приходу стакана проверяем, активен ли инструмент
           Тут надо еще проверить, нет ли запросов по той же фиге.
           Отсылку запроса на Order также делаем в случае прихода ответа.
           Если ответ удачный - переходим к следующему запросу.
           Если неудачный - повторяем, но не больше TryMax

           Счетчики повтора храним в мапе по фиге - там лежит счётчик для текущего инструмента

           Номер следующего запроса также храним в мапе по фиге. Все запросы по фиге считаются исполненными,
           когда этот номер равен или больше размеру вектора запросов по фиге.


        5) Если активен, отсылаем запрос. Тут надо связать запрос с OrderParams, из которых он строится.

           Храним FIGI, индекс в массиве, привязанном к FIGI, и response (QSharedPointer),
           кладём это в какой-то вектор

     */


    typedef std::size_t index_t;

    const index_t MaxTryes = 5;


    std::map< QString, std::vector<tkf::OrderParams> >             figiOrders;
    std::map< QString, index_t >                                   tryCounters;
    std::map< QString, index_t >                                   nextOrders;
    std::map< QString, QSharedPointer<tkf::LimitOrderResponse> >   awaitingResponses;


    cout << endl;

    cout << "Requested orders:" << endl;


    for( auto orderParams : inputOrderParams )
    {
        auto figi = orderParams.figi;

        figiOrders [figi].push_back(orderParams);
        tryCounters[figi] = 0;
        nextOrders [figi] = 0;

        cout << "  " << orderParams.to_string() << endl;
    }

    cout << endl;

    return 0;
    

    auto checkIsAllComplete = [&]()
                              {
                                  bool allComplete = true;

                                  std::map< QString, std::vector<tkf::OrderParams> >::const_iterator figiIt = figiOrders.begin();
                                  for(; figiIt != figiOrders.end(); ++figiIt )
                                  {

                                      const auto &figi      = figiIt->first;
                                      auto  figiTotalOrders = figiIt->second.size();
                                      auto  figiNextOrderNo = nextOrders[figi];

                                      if (figiNextOrderNo>=figiTotalOrders)
                                      {
                                          allComplete = false;
                                          break;
                                      }

                                  }

                                  return allComplete;

                              };







    std::vector< std::string > argsVec;
    for( int i=1; i<argc; ++i)
        argsVec.push_back(argv[i]);


    tkf::OrderParams orderParams;

    int orderParamsParsingRes = tkf::parseOrderParams( argsVec, dicts, orderParams );
    if (orderParamsParsingRes<0)
    {
        std::cerr << "Error: invalid argument #" << -orderParamsParsingRes << endl;
        return -orderParamsParsingRes;
    }


    cout << endl;
    cout << endl;
    cout << "Order Parameters:" << endl;

    cout << "  Operation  : " << orderParams.getOperationTypeString() << endl;
    cout << "  Order Type : " << orderParams.getOrderTypeString() << endl;
    cout << "  Instrument : " << orderParams.figi << " (" << orderParams.ticker << ")" << endl;
    cout << "  Order Size : " << (int)orderParams.orderSize << " (pieces, not lots)" << endl;
    if (orderParams.orderPrice!=0)
    {
        cout << "  Price      : " << orderParams.orderPrice << endl;
    }

    cout << endl;
    cout << endl;


    tkf::MarketInstrumentState   instrumentState; // volatile
    tkf::MarketGlass             instrumentGlass; // volatile


    //return 0;




    QWebSocket webSocket;
    console_helpers::SimpleHandleCtrlC ctrlC; // ctrlC.isBreaked()


    std::atomic<bool> fConnected = false;


    auto onConnected = [&]()
            {
                using std::cout;  using std::endl;
            
                fConnected.store( true, std::memory_order_seq_cst  );

                cout << "# Streaming API Web socket connected" << endl;

                QString orderBookSubscriptionText         = pOpenApi->getStreamingApiOrderbookSubscribeJson( orderParams.figi );
                QString instrumentInfoSubscriptionText    = pOpenApi->getStreamingApiInstrumentInfoSubscribeJson( orderParams.figi );

                QTest::qWait(5);
                cout << "# Subscribe to instrument info for " << orderParams.ticker << " (" << orderParams.figi << ")" << endl;
                //cout << "# Message: "   << instrumentInfoSubscriptionText << endl;
                webSocket.sendTextMessage( instrumentInfoSubscriptionText );

                QTest::qWait(5);
                cout << "# Subscribe to orderbook for " << orderParams.ticker << " (" << orderParams.figi << ")" << endl;
                //cout << "# Message: "   << orderBookSubscriptionText << endl;
                webSocket.sendTextMessage( orderBookSubscriptionText );

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

                    if (marketGlass.isValid() && marketGlass.figi==orderParams.figi)
                        instrumentGlass = marketGlass;

                }

                else if (eventName=="instrument_info")
                {
                    tkf::StreamingInstrumentInfoResponse response;
                    response.fromJson(msg);

                    tkf::MarketInstrumentState instrState = tkf::MarketInstrumentState::fromStreamingInstrumentInfoResponse( response );

                    if (instrState.isValid() && instrState.figi==orderParams.figi)
                    {
                        instrumentState = instrState;
                        //cout << "# !!! Streaming error: got an invalid MarketInstrumentState - " << msg << endl;
                    }
                }

                else if (eventName=="candle")
                {
                    //cout << "# !!! Candle event not handled, data: " << endl << msg << endl << endl;
                }

                else
                {
                    //cout << "# !!! Unknown event: " << eventName << endl;
                    //cout << msg << endl;
                }

                // std::map< QString, tkf::MarketInstrumentState >  marketInstrumentState;

                // https://bcs-express.ru/novosti-i-analitika/o-chem-mogut-rasskazat-birzhevoi-stakan-i-lenta-sdelok



            };


    cout << "# Connecting Streaming API Web socket" << endl;
    webSocket.connect( &webSocket, &QWebSocket::connected             , onConnected    );
    webSocket.connect( &webSocket, &QWebSocket::disconnected          , onDisconnected );
    webSocket.connect( &webSocket, &QWebSocket::textMessageReceived   , onMessage      );

    webSocket.open( pOpenApi->getStreamingApiNetworkRequest() );


    //cout << "# Press Ctrl+C to break process" << endl;

    //cout<<"#" << endl;

    cout << endl;
    cout << endl;


    QElapsedTimer timer;
    timer.start();

    while(!ctrlC.isBreaked() && !timer.hasExpired(30000) && !(instrumentState.isValid() && instrumentGlass.isValid()) )
    {
        QTest::qWait(1);
    }

    if (instrumentState.isValid() && instrumentGlass.isValid())
    {
        cout << "Got instrument streaming info" << endl;

        if (!instrumentState.isTradeStatusNormalTrading())
        {
            cout << "Instrument not available for trading" << endl;
        }
        else if (instrumentState.lotSize==0)
        {
            cout << "Instrument lot size is invalid" << endl;
        }
        else if (instrumentState.priceIncrement==0)
        {
            cout << "Instrument price increment is invalid" << endl;
        }
        else
        {

            unsigned numLots = orderParams.calcNumLots( instrumentState.lotSize );

            orderParams = orderParams.getAdjusted( instrumentGlass.getPriceSpreadPoints(instrumentState.priceIncrement)
                                                 , instrumentState.priceIncrement
                                                 , instrumentGlass.getAsksMinPrice()
                                                 , instrumentGlass.getBidsMaxPrice()
                                                 );

            cout << endl;
            cout << endl;

            cout << "Order Parameters (adjusted):" << endl;
            cout << "  Operation : " << orderParams.getOperationTypeString() << endl;
            cout << "  Order Type: " << orderParams.getOrderTypeString() << endl;
            //cout << "  Instrument: " << orderParams.figi << " (" << orderParams.ticker << ")" << endl;
            cout << "  Order Size: " << (int)orderParams.orderSize << " pieces, " << numLots << " lots" << endl;
            if (orderParams.orderPrice!=0)
            {
                cout << "  Price     : " << orderParams.orderPrice << endl;
            }

            cout << endl;
            cout << endl;

            tkf::PlacedOrderInfo   placedOrderInfo;


            if (orderParams.isOrderTypeLimit())
            {
                auto response = pOpenApi->ordersLimitOrder( orderParams.figi
                                                          , orderParams.getOpenApiOperationType()
                                                          , numLots
                                                          , orderParams.orderPrice
                                                          );
                response->join();
                tkf::checkAbort(response);
                placedOrderInfo = tkf::PlacedOrderInfo::fromOrderResponse(response->value);

            }
            else
            {
                auto response = pOpenApi->ordersMarketOrder( orderParams.figi
                                                          , orderParams.getOpenApiOperationType()
                                                          , numLots
                                                          );
                response->join();
                tkf::checkAbort(response);
                placedOrderInfo = tkf::PlacedOrderInfo::fromOrderResponse(response->value);
                            
            }

            cout << "Response:" << endl;
            cout << endl;

            cout << "  TrackingId     : " << placedOrderInfo.trackingId << endl;
            cout << "  Status         : " << placedOrderInfo.status << endl;
            cout << endl;

            cout << "  OrderId        : " << placedOrderInfo.orderId << endl;
            cout << "  Order Status   : " << placedOrderInfo.orderStatus << endl;
            cout << "  Operation Type : " << placedOrderInfo.operationType << endl;
            cout << endl;

            cout << "  Requested Lots : " << placedOrderInfo.requestedLots << endl;
            cout << "  Executed  Lots : " << placedOrderInfo.executedLots << endl;
            cout << "  Commission     : " << placedOrderInfo.commission << endl;
            cout << endl;

            if (!placedOrderInfo.rejectReason.isEmpty())
            {
                cout << "  Reject Reason  : " << placedOrderInfo.rejectReason << endl;
                cout << endl;
            }

            if (!placedOrderInfo.message.isEmpty())
            {
                cout << "  Message        : " << placedOrderInfo.message << endl;
                cout << endl;
            }




/*

    TKF_IOA_ABSTRACT_METHOD( LimitOrderResponse , ordersLimitOrder (const QString &figi, const LimitOrderRequest  &limit_order_request , QString broker_account_id = QString()) );
    TKF_IOA_ABSTRACT_METHOD( LimitOrderResponse , ordersLimitOrder (const QString &figi, const OperationType &operation, qint32 nLots  , marty::Decimal price, QString broker_account_id = QString()) );
    TKF_IOA_ABSTRACT_METHOD( LimitOrderResponse , ordersLimitOrder (const QString &figi, const OperationType::eOperationType &operation, qint32 nLots, marty::Decimal price, QString broker_account_id = QString()) );
    TKF_IOA_ABSTRACT_METHOD( LimitOrderResponse , ordersLimitOrder (const QString &figi, const QString &operation      , qint32 nLots  , marty::Decimal price, QString broker_account_id = QString()) );

    TKF_IOA_ABSTRACT_METHOD( MarketOrderResponse, ordersMarketOrder(const QString &figi, const MarketOrderRequest &market_order_request, QString broker_account_id = QString()) );
    TKF_IOA_ABSTRACT_METHOD( MarketOrderResponse, ordersMarketOrder(const QString &figi, const OperationType &operation, qint32 nLots  , QString broker_account_id = QString()) );
    TKF_IOA_ABSTRACT_METHOD( MarketOrderResponse, ordersMarketOrder(const QString &figi, const OperationType::eOperationType &operation, qint32 nLots, QString broker_account_id = QString()) );
    TKF_IOA_ABSTRACT_METHOD( MarketOrderResponse, ordersMarketOrder(const QString &figi, const QString &operation      , qint32 nLots  , QString broker_account_id = QString()) );


*/       
        }

        //
    }




    if (fConnected.load()!=false)
    {
       //cout << "# WebSocket forced closing" << endl;

       webSocket.close();

       while(fConnected.load()!=false)
       {
           QTest::qWait(1);
       }

       //cout << "# WebSocket closed" << endl;

    }

/*
    connect(&m_webSocket, &QWebSocket::connected, this, &EchoClient::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &EchoClient::closed);

    connect(&m_webSocket, &QWebSocket::textMessageReceived,
            this, &EchoClient::onTextMessageReceived);
*/
    
    return 0;
}



