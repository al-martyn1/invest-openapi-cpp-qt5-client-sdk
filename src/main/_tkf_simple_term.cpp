/*! \file
    \brief 
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
#include <deque>

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

#include "invest_openapi/openapi_limits.h"

#include "invest_openapi/console_break_helper.h"


#include "invest_openapi/streaming_handlers.h"

#include "invest_openapi/market_glass.h"
#include "invest_openapi/market_instrument_state.h"

#include "invest_openapi/format_helpers.h"

#include "invest_openapi/term_helpers.h"




INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("_tkf_simple_term");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;

    // cout<<"# Path to exe   : "<<QCoreApplication::applicationDirPath().toStdString()<<endl;

    // cout << "#" << endl;

    namespace tkf=invest_openapi;
    using tkf::config_helpers::lookupForConfigFile;
    using tkf::config_helpers::FileReadable;

    QStringList lookupConfSubfolders = QString("conf;config").split( ';', Qt::SkipEmptyParts );

    auto logConfigFullFileName     = lookupForConfigFile( "logging.properties" , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto apiConfigFullFileName     = lookupForConfigFile( "config.properties"  , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto authConfigFullFileName    = lookupForConfigFile( "auth.properties"    , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto dbConfigFullFileName      = lookupForConfigFile( "database.properties", lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto instrumentsConfigFullFileName = lookupForConfigFile( "instruments.properties" , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );

    // cout << "# Log  Config File: "<< logConfigFullFileName   << endl;
    // cout << "# API  Config File: "<< apiConfigFullFileName   << endl;
    // cout << "# Auth Config File: "<< authConfigFullFileName  << endl;
    // cout << "# Instruments Cfg File: "<< instrumentsConfigFullFileName << endl;

    auto apiConfig     = tkf::ApiConfig    ( apiConfigFullFileName  );
    auto authConfig    = tkf::AuthConfig   ( authConfigFullFileName );

    QSharedPointer<tkf::DatabaseConfig> pDatabaseConfig = QSharedPointer<tkf::DatabaseConfig>( new tkf::DatabaseConfig(dbConfigFullFileName, tkf::DatabasePlacementStrategyDefault()) );

    QSharedPointer<tkf::LoggingConfig>  pLoggingConfig  = QSharedPointer<tkf::LoggingConfig> ( new tkf::LoggingConfig(logConfigFullFileName) );

    pLoggingConfig->debugSqlQueries = false;

    auto loggingConfig = *pLoggingConfig;



    auto dataLogFullFilename = pLoggingConfig->getDataLogFullName( logConfigFullFileName, "", "test.dat" );

    // cout << "# test_streaming_api data log file: "<< dataLogFullFilename << endl;




    // cout << "# Main DB name: " << pDatabaseConfig->dbMainFilename << endl;

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


    QStringList instrumentList;
    {
        QSettings settings(instrumentsConfigFullFileName, QSettings::IniFormat);
        instrumentList = settings.value("instruments" ).toStringList();
    }



    // Custom code goes here

    // Key - FIGI
    std::map< QString, tkf::MarketInstrumentState >     instrumentStates;
    std::map< QString, tkf::MarketGlass           >     instrumentGlasses;
    std::map< QString, std::vector< tkf::Operation > >  instrumentOperations;




    auto isInstrumentActive         = [&]( QString figi )
                                      {
                                          figi = dicts.findFigiByAnyIdString(figi); // This allows to use ticker also (for test)

                                          std::map< QString, tkf::MarketInstrumentState >::const_iterator it = instrumentStates.find(figi);
                                          if (it == instrumentStates.end())
                                              return false;
                                   
                                          return it->second.isTradeStatusNormalTrading();
                                      };



    QWebSocket webSocket;
    console_helpers::SimpleHandleCtrlC ctrlC; // ctrlC.isBreaked()


    std::atomic<bool> fConnected = false;

    QString statusStr;


    auto printFigiInfoLine = [&]( QString figi )
                             {
                                 cout << tkf::format_field( 0 /* leftSpace */ , 2 /* rightSpace */ , 12 /* fieldWidth */ , -1, figi );

                                 figi = dicts.findFigiByAnyIdString(figi);

                                 if (!isInstrumentActive(figi))
                                 {
                                     cout << "Innactive" << endl;
                                     return;
                                 }

                                 /*
                                    Текущая цена
                                    Цена последней продажы
                                    Цена последней покупки
                                    Ближайшая заявка на продажу
                                    Ближайшая заявка на покупку
                                 
                                  */

                             };



    auto updateScreen =      [&]( )
                             {
                                 cout << "Status: " << statusStr << endl;

                                 cout << endl;

                                 for( auto figi : instrumentList )
                                 {
                                     //QString figi = dicts.findFigiByAnyIdString(figi);

                                     printFigiInfoLine( figi );
                                 }

                                 cout << endl;

                             };



    auto updateFigiScreen =  [&]( QString figi )
                             {
                                 // Do something for FIGI

                                 updateScreen();

                             };



    auto onConnected = [&]()
            {
                using std::cout;  using std::endl;
            
                fConnected.store( true, std::memory_order_seq_cst  );

                //std::map< QString, std::vector<tkf::OrderParams> >::const_iterator it = figiOrders.begin();
                for( auto figi : instrumentList )
                {
                    //cout << "onConnected: figi: <" << figi << ">, size: " << figi.size() << endl;

                    figi = dicts.findFigiByAnyIdString(figi);

                    QString orderBookSubscriptionText         = pOpenApi->getStreamingApiOrderbookSubscribeJson( figi );
                    QString instrumentInfoSubscriptionText    = pOpenApi->getStreamingApiInstrumentInfoSubscribeJson( figi );
                   
                    QTest::qWait(5);
                    webSocket.sendTextMessage( instrumentInfoSubscriptionText );
                   
                    QTest::qWait(5);
                    webSocket.sendTextMessage( orderBookSubscriptionText );
                }


                statusStr = "Connected";

                updateScreen();

            };

    auto onDisconnected = [&]()
            {
                using std::cout;  using std::endl;
            
                fConnected.store( false, std::memory_order_seq_cst  );

                // Try to reconnect

                webSocket.open( pOpenApi->getStreamingApiNetworkRequest() );

                statusStr = "Disconnected";

                updateScreen();
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

                    statusStr = QString("Streaming error: ") + streamingError.getPayload().getMessage();

                    //updateFigiScreen(marketGlass.figi);
                    updateScreen();
                }

                else if (eventName=="orderbook")
                {
                    tkf::StreamingOrderbookResponse response;
                    response.fromJson(msg);

                    tkf::MarketGlass marketGlass = tkf::MarketGlass::fromStreamingOrderbookResponse(response);

                    instrumentGlasses[marketGlass.figi] = marketGlass;

                    updateFigiScreen(marketGlass.figi);
                }

                else if (eventName=="instrument_info")
                {
                    tkf::StreamingInstrumentInfoResponse response;
                    response.fromJson(msg);

                    tkf::MarketInstrumentState instrState = tkf::MarketInstrumentState::fromStreamingInstrumentInfoResponse( response );

                    instrumentStates[instrState.figi] = instrState;

                    updateFigiScreen(instrState.figi);
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





    //     auto foundOpResponseIt = tkf::findOpenApiCompletableFutureFinished( operationResponses.begin(), operationResponses.end() );
    //     if (foundOpResonseIt!=operationResponses.end())
    //     {
    //         auto operationResponse = *foundOpResonseIt;
    //  
    //         operationResponses.erase(foundOpResonseIt);
    //  
    //  
    // QString getFigi() const;

    // QList< kf::Operation > getOperations() const;
    // std::map< QString, sd::vector< kf::Operation > >  instrumentOperations;

    std::vector< QSharedPointer< tkf::OpenApiCompletableFuture< tkf::OperationsResponse > > > awaitingOperationResponses;
    std::set< QString > awaitingOperationResponsesFigi; // Набор фиг по текущим запросам по операциям. 

    auto checkAwaitingOperationResponses = [&]()
                                           {
                                               auto foundOpResponseIt = tkf::findOpenApiCompletableFutureFinished( awaitingOperationResponses.begin(), awaitingOperationResponses.end() );
                                               if (foundOpResponseIt==awaitingOperationResponses.end())
                                                   return;

                                               auto operationsResponse = *foundOpResponseIt;
                                              
                                               awaitingOperationResponses.erase(foundOpResponseIt);

                                               //operationsResponseResult = operationsResponse->result();

                                               if (!operationsResponse /* Result */ ->isResultValid())
                                               {
                                                   statusStr = QString("Get Operations Error: ") + operationsResponse->getErrorMessage();
                                                   updateScreen();
                                                   return;
                                               }

                                               //auto payload = 

                                               std::map< QString, std::vector< tkf::Operation > > tmpOperationsFigiMap;

                                               QList< tkf::Operation > operations = operationsResponse->value.getPayload().getOperations();

                                               for( const auto &op : operations )
                                               {
                                                   QString figi = op.getFigi().toUpper();

                                                   QString operationStatusStr = op.getStatus().asJson().toUpper();
                                                   if (operationStatusStr!="DONE" && operationStatusStr!="PROGRESS") // Also may be DECLINE or INVALID_...
                                                       continue;
                                                   
                                                   QString operationTypeStr   = op.getOperationType().asJson().toUpper();
                                                   if (operationTypeStr!="BUYCARD" && operationTypeStr!="BUY" && operationTypeStr!="SELL")
                                                       continue;

                                                   //opVec.push_back(op);
                                                   tmpOperationsFigiMap[figi].push_back(op);
                                               }


                                               std::map< QString, std::vector< tkf::Operation > >::iterator it = tmpOperationsFigiMap.begin();
                                               for( ; it!=tmpOperationsFigiMap.end(); ++it )
                                               {
                                                   const QString &figi = it->first;
                                                   auto &opVec = it->second;

                                                   std::stable_sort( opVec.begin(), opVec.end()
                                                                   , []( const tkf::Operation &op1, const tkf::Operation &op2 )
                                                                     {
                                                                         return op1.getDate() < op2.getDate();
                                                                     }
                                                                   );

                                                   instrumentOperations[figi] = opVec;

                                                   /*
                                                   cout << "--------" << endl;
                                                   QString ticker = dicts.getTickerByFigiChecked(figi);
                                                   cout << "Operations for Figi: " << figi << " (" << ticker << ")" << endl;

                                                   auto opIt = opVec.begin();
                                                   for(; opIt!=opVec.end(); ++opIt)
                                                   {
                                                       cout << "Operation: " << opIt->getOperationType().asJson().toUpper() << endl;
                                                       cout << "Status   : " << opIt->getStatus().asJson().toUpper() << endl;
                                                       cout << "Date&Time: " << opIt->getDate() << endl;
                                                       cout << "Trades # :"  << opIt->getTrades().size() << endl;
                                                       cout << endl;
                                                   } // for(; opIt!=opVec.end(); ++opIt)
                                                   */

                                               } // for( ; it!=tmpOperationsFigiMap.end(); ++it )
                                           
                                           };


    
    QSharedPointer< tkf::OpenApiCompletableFuture < tkf::OrdersResponse > > ordersResponse  = pOpenApi->orders();

    QStringList::const_iterator instrumentForOperationsIt = instrumentList.begin();
    

    /*
        Делать запрос по ордерам один раз в конце списка инструментов - не дело.
        
        По операции инструментам неспешно полируем с паузой в пару секунд (и один раз при старте, 
        до главного цикла).

        А надо ли вообще полировать?

        Наверное, каждый раз надо и ордера запрашивать. Если пауза - 3 секунды, и два запроса, 
        то это 40 запросов в минуту (если 5 - то 24), при лимите в 120. 
        При ручном добавлении заявок - вполне хватит. Для робота, и если куча инструментов - 
        наверное маловато будет.

        Текущие ордера надо хранить. Если пришел ответ, и там ордера нет, но есть локально - 
        значит, ордер исполнен, и надо запросить операции по инструменту.

        Теоретически, если запрос по операциям вернул ошибку - надо бы перезапросить

    
     */

    QElapsedTimer operationsRequestTimer;
    operationsRequestTimer.start();

    std::size_t requestCounter = 0;

    for(; instrumentForOperationsIt!=instrumentList.end(); ++instrumentForOperationsIt, ++requestCounter)
    {
        tkf::waitOnRequestsLimit( operationsRequestTimer, requestCounter );

        QDateTime dateTimeNow     = QDateTime::currentDateTime();
        QDateTime dateTimeBefore  = qt_helpers::dtAddTimeInterval( dateTimeNow, QString("-10YEAR") );

        QString requestForFigi    = dicts.findFigiByAnyIdString(*instrumentForOperationsIt);
        
        auto operationsResponse   = pOpenApi->operations(dateTimeBefore, dateTimeNow, requestForFigi);

        awaitingOperationResponses.push_back(operationsResponse);

        checkAwaitingOperationResponses();

    }

    while(!awaitingOperationResponses.empty())
        checkAwaitingOperationResponses();




    //cout << "# Connecting Streaming API Web socket" << endl;
    webSocket.connect( &webSocket, &QWebSocket::connected             , onConnected    );
    webSocket.connect( &webSocket, &QWebSocket::disconnected          , onDisconnected );
    webSocket.connect( &webSocket, &QWebSocket::textMessageReceived   , onMessage      );

    webSocket.open( pOpenApi->getStreamingApiNetworkRequest() );




    while(!ctrlC.isBreaked() )
    {
        QTest::qWait(1);

        #if 0

        if (operationsRequestTimer.hasExpired(500))
        {
            operationsRequestTimer.restart();

            if (instrumentForOperationsIt==instrumentList.end())
            {
                instrumentForOperationsIt = instrumentList.begin();
                //ordersResponse = pOpenApi->orders();
                // if (ordersResponse->isFinished())
                // {
                //     ordersResponse = pOpenApi->orders(); // send new request for orders
                // }
            }
            else
            {
                QDateTime dateTimeNow     = QDateTime::currentDateTime();
                QDateTime dateTimeBefore  = qt_helpers::dtAddTimeInterval( dateTimeNow, QString("-10YEAR") );

                QString requestForFigi    = dicts.findFigiByAnyIdString(*instrumentForOperationsIt);
                
                auto operationsResponse   = pOpenApi->operations(dateTimeBefore, dateTimeNow, requestForFigi);
            
            }

        }


        if (ordersResponse->isFinished())
        {
            // auto responseValue = ordersResponse->value;

    // QList<tkf::Order> orders = responseValue.getPayload();
    //  
    // for( auto order : orders )
    // {
    //     cout << "----------------------" << endl << endl;
    //  
    //     QString figi   = order.getFigi();
    //     QString ticker = dicts.getTickerByFigiChecked(figi);
    //  
    //     cout << "Instrument     : " << ticker << "/" << figi    << endl;
    //  
    //     cout << "Order Id       : " << order.getOrderId()       << endl;
    //     cout << "Operation      : " << order.getOperation()     << endl;
    //     cout << "Status         : " << order.getStatus()        << endl;
    //     cout << "Order Type     : " << order.getType()          << endl;
    //  
    //     cout << endl;
    //  
    //     cout << "Lots Requested : " << order.getRequestedLots() << endl;
    //     cout << "Lots Executed  : " << order.getExecutedLots() << endl;
    //  
    //     cout << endl;
    //  
    //     cout << "Price          : " << order.getPrice() << endl;


        }


        auto foundOpResponseIt = tkf::findOpenApiCompletableFutureFinished( awaitingOperationResponses.begin(), awaitingOperationResponses.end() );
        if (foundOpResonseIt!=awaitingOperationResponses.end())
        {
            auto operationResponse = *foundOpResonseIt;

            awaitingOperationResponses.erase(foundOpResonseIt);

            // auto responseValue = ordersResponse->value;
        // tkf::checkAbort(operationsResponse);
        //  
        // // QList<Operation> getOperations() const;
        // auto operations = operationsResponse->value.getPayload().getOperations();


        // for( auto op : operations )
        // {
        //     // QList<OperationTrade> op.getTrades() const;
        //     // OperationTypeWithCommission getOperationType() const;
        //  
        //     QString operationTypeStr   = op.getOperationType().asJson().toUpper();
        //     QString operationStatusStr = op.getStatus().asJson().toUpper();
        //  
        //     if (operationStatusStr!="DONE" && operationStatusStr!="PROGRESS") // DECLINE or INVALID
        //         continue;
        //  
        //     if (operationTypeStr!="BUYCARD" && operationTypeStr!="BUY" && operationTypeStr!="SELL")
        //         continue;
        //  
        //     cout << "Operation: " << operationTypeStr << endl;
        //     cout << "Status   : " << operationStatusStr << endl;
        //     cout << "Trades   :"  << endl;
        //  
        //  
        //     auto trades = op.getTrades(); // QList<OperationTrade>
        //  
        //     for( auto trade : trades )
        //     {
        //         cout << "    " << trade.getQuantity() << " x " << trade.getPrice() << endl;
        //     }


        }

        //IteratorType findOpenApiCompletableFutureFinished( IteratorType b, IteratorType e )

        //if (instrumentForOperationsIt

        #endif
    }




        // operationsResponse->join();
        //  
        // tkf::checkAbort(operationsResponse);
        //  
        // // QList<Operation> getOperations() const;
        // auto operations = operationsResponse->value.getPayload().getOperations();

            // QElapsedTimer timer;
            // timer.start();
            // auto sandboxRegisterResponseInterval = timer.restart();
    

    return 0;

    
}



