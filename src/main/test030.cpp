/*! \file
    \brief Тест Тинькофф Streaming API - подписка на стаканы, свечи, инструменты инфу - с дублем, и потом отписка. Тестируем разбор приходящего JSON'а

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


INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("test030");
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
    //auto balanceConfigFullFileName = lookupForConfigFile( "balance.properties" , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );

    qDebug().nospace().noquote() << "Log  Config File: "<< logConfigFullFileName  ;
    qDebug().nospace().noquote() << "API  Config File: "<< apiConfigFullFileName  ;
    qDebug().nospace().noquote() << "Auth Config File: "<< authConfigFullFileName ;

    auto apiConfig     = tkf::ApiConfig    ( apiConfigFullFileName  );
    auto authConfig    = tkf::AuthConfig   ( authConfigFullFileName );

    QSharedPointer<tkf::LoggingConfig>  pLoggingConfig  = QSharedPointer<tkf::LoggingConfig> ( new tkf::LoggingConfig(logConfigFullFileName) );
    auto loggingConfig = *pLoggingConfig;



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

    // https://tinkoffcreditsystems.github.io/invest-openapi/marketdata/
    // wss://api-invest.tinkoff.ru/openapi/md/v1/md-openapi/ws
    // wss://api-invest.tinkoff.ru/openapi/md/v1/md-openapi/ws
    // c, _, err := websocket.DefaultDialer.Dial(*addr, http.Header{"Authorization": {"Bearer " + *token}})

    qDebug().nospace().noquote()<<"";
    qDebug().nospace().noquote()<<"";

    QWebSocket webSocket;
    console_helpers::SimpleHandleCtrlC ctrlC; // ctrlC.isBreaked()


    typedef std::pair< QString,QString >  figi_info_pair_t;

    std::vector< figi_info_pair_t > figis = { { "BBG000BN56Q9" ,   "DSKY"           }
                                            , { "BBG004731354" ,   "ROSN"           }
                                            , { "BBG004731354" ,   "ROSN"           }
                                            , { "BBG0047315Y7" ,   "SBERP"          }
                                            };

    // https://habr.com/ru/post/517918/


    std::atomic<bool> fConnected = false;
    std::atomic<bool> jobDone    = false;

    auto onConnected = [&]()
            {
                using std::cout;
                using std::endl;
            
                fConnected.store( true, std::memory_order_seq_cst  );

                cout << "*** Streaming API Web socket connected" << endl;
                cout << endl;

                QString figi = "BBG000BN56Q9", ticker = "DSKY";
                QString subscriptionText = pOpenApi->getStreamingApiOrderbookSubscribeJson( figi, -1 ); // Generate
                
                cout << "/// Try to subscribe to orderbook with error in request" << endl;
                cout << "Subscription text:" << endl << subscriptionText << endl << "--------" << endl << endl;
                webSocket.sendTextMessage( subscriptionText );

            };

    auto onDisconnected = [&]()
            {
                using std::cout;
                using std::endl;
            
                fConnected.store( false, std::memory_order_seq_cst  );

                cout << "*** Streaming API Web socket disconnected" << endl;
                cout << endl;

            };

    auto onMessage = [&]( QString msg )
            {
                using std::cout;
                using std::endl;
            
                // cout << "!!! Streaming API Web socket received message: " << endl<< msg << endl << "--------" << endl << endl;
                tkf::GenericStreamingResponse genericStreamingResponse;
                genericStreamingResponse.fromJson(msg);

                auto streamingEvent = genericStreamingResponse.getEvent();
                if (streamingEvent=="error")
                {
                    tkf::StreamingError streamingError;
                    streamingError.fromJson(msg);
                    cout << "!!! Streaming event: error, message: " << streamingError.getPayload().getMessage() << endl;
                }
                else if (streamingEvent=="orderbook")
                {
                    tkf::StreamingOrderbookResponse streamingOrderbookResponse;
                    streamingOrderbookResponse.fromJson(msg);

                    cout << "!!! Streaming event: orderbook" << endl;
                }
                else if (streamingEvent=="candle")
                {
                    tkf::StreamingCandleResponse  streamingCandleResponse;
                    streamingCandleResponse.fromJson(msg);

                    cout << "!!! Streaming event: candle" << endl;
                }
                else if (streamingEvent=="instrument_info")
                {
                    tkf::StreamingMarketInstrumentResponse  streamingMarketInstrumentResponse;
                    streamingMarketInstrumentResponse.fromJson(msg);

                    cout << "!!! Streaming event: instrument_info" << endl;
                }
                else
                {
                    cout << "??? Unknown streaming event: " << genericStreamingResponse.getEvent() << endl << "Message:" << endl << msg << endl;
                }




            };


    // https://wiki.qt.io/New_Signal_Slot_Syntax

    webSocket.connect( &webSocket, &QWebSocket::connected             , onConnected    );
    webSocket.connect( &webSocket, &QWebSocket::disconnected          , onDisconnected );
    webSocket.connect( &webSocket, &QWebSocket::textMessageReceived   , onMessage      );

    webSocket.open( pOpenApi->getStreamingApiNetworkRequest() );


    cout << endl;
    cout << endl;
    cout << "Press Ctrl+C to break process" << endl;
    cout << endl;


    std::vector< figi_info_pair_t >::const_iterator figiIt  = figis.begin();
    std::vector< figi_info_pair_t >::const_iterator figiEnd = figis.end  ();

    QElapsedTimer stopTimer   ;  stopTimer   .start();
    QElapsedTimer requestTimer;  requestTimer.start();

    const std::uint64_t requestPeriod = 3000; // ms
    const std::uint64_t stopTimeout =  /* figis.size()*5*requestPeriod + */  60*1000; // Ждём 60 сек после того, как закончатся фиги туда-сюда

    #define IF_FIGI_ITERATOR_NOT_END()                               \
                 if (requestTimer.elapsed() < requestPeriod)         \
                     break;                                          \
                                                                     \
                 if (figiIt==figiEnd)                                \
                 {                                                   \
                     figiIt = figis.begin();                         \
                     state  = (State)(state+1);                      \
                     requestTimer.restart();                         \
                     break;                                          \
                 }                                                   \
                 else

    #define CASE_STATE( st ) case st : IF_FIGI_ITERATOR_NOT_END()

    #define CASE_BREAK()                     \
                     ++figiIt;               \
                     requestTimer.restart(); \
                     break


    enum State
    {
        stateSubscribeFigis,
        stateSubscribeCandles1min,
        stateSubscribeCandles5min,
        stateSubscribeInstrumentInfo,
        statePause,
        stateUnsubscribeFigis,
        stateUnsubscribeCandles1min,
        stateUnsubscribeCandles5min,
        stateUnsubscribeInstrumentInfo,
        stateFinal

    };


    State state = stateSubscribeFigis;


    while(!ctrlC.isBreaked())
    {
        QTest::qWait(1);

        switch(state)
        {
            CASE_STATE(stateSubscribeFigis)
                 {
                     QString figi = figiIt->first, ticker = figiIt->second;
                     QString subscriptionText = pOpenApi->getStreamingApiOrderbookSubscribeJson( figi );
                
                     cout << "/// Try to subscribe to orderbook for - FIGI: " << figi << ", TICKER: " << ticker << endl;
                     cout << "Subscription text:" << endl << subscriptionText << endl << "--------" << endl << endl;
                     webSocket.sendTextMessage( subscriptionText );

                     CASE_BREAK();
                 }
                 
            CASE_STATE(stateSubscribeCandles1min)
                 {
                     QString figi = figiIt->first, ticker = figiIt->second;
                     QString subscriptionText = pOpenApi->getStreamingApiCandleSubscribeJson( figi, "1MIN" );
                
                     cout << "/// Try to subscribe to 1 min candles for - FIGI: " << figi << ", TICKER: " << ticker << endl;
                     cout << "Subscription text:" << endl << subscriptionText << endl << "--------" << endl << endl;
                     webSocket.sendTextMessage( subscriptionText );

                     CASE_BREAK();
                 }
                 
            CASE_STATE(stateSubscribeCandles5min)
                 {
                     QString figi = figiIt->first, ticker = figiIt->second;
                     QString subscriptionText = pOpenApi->getStreamingApiCandleSubscribeJson( figi, "5MIN" );
                
                     cout << "/// Try to subscribe to 5 min candles for - FIGI: " << figi << ", TICKER: " << ticker << endl;
                     cout << "Subscription text:" << endl << subscriptionText << endl << "--------" << endl << endl;
                     webSocket.sendTextMessage( subscriptionText );

                     CASE_BREAK();
                 }

            CASE_STATE(stateSubscribeInstrumentInfo)
                 {
                     QString figi = figiIt->first, ticker = figiIt->second;
                     QString subscriptionText = pOpenApi->getStreamingApiInstrumentInfoSubscribeJson( figi );
                
                     cout << "/// Try to subscribe to instrument info for - FIGI: " << figi << ", TICKER: " << ticker << endl;
                     cout << "Subscription text:" << endl << subscriptionText << endl << "--------" << endl << endl;
                     webSocket.sendTextMessage( subscriptionText );

                     CASE_BREAK();
                 }

            CASE_STATE(statePause)
                 {
                     // do nothing with FIGIs
                     CASE_BREAK();
                 }
                 
            CASE_STATE(stateUnsubscribeFigis)
                 {
                     QString figi = figiIt->first, ticker = figiIt->second;
                     QString subscriptionText = pOpenApi->getStreamingApiOrderbookUnsubscribeJson( figi );
                
                     cout << "/// Try to unsubscribe to orderbook for - FIGI: " << figi << ", TICKER: " << ticker << endl;
                     cout << "Unsubscription text:" << endl << subscriptionText << endl << "--------" << endl << endl;
                     webSocket.sendTextMessage( subscriptionText );

                     CASE_BREAK();
                 }
                 
            CASE_STATE(stateUnsubscribeCandles1min)
                 {
                     QString figi = figiIt->first, ticker = figiIt->second;
                     QString subscriptionText = pOpenApi->getStreamingApiCandleUnsubscribeJson( figi, "1MIN" );
                
                     cout << "/// Try to unsubscribe to 1 min candles for - FIGI: " << figi << ", TICKER: " << ticker << endl;
                     cout << "Unsubscription text:" << endl << subscriptionText << endl << "--------" << endl << endl;
                     webSocket.sendTextMessage( subscriptionText );

                     CASE_BREAK();
                 }
                 
            CASE_STATE(stateUnsubscribeCandles5min)
                 {
                     QString figi = figiIt->first, ticker = figiIt->second;
                     QString subscriptionText = pOpenApi->getStreamingApiCandleUnsubscribeJson( figi, "5MIN" );
                
                     cout << "/// Try to unsubscribe to 5 min candles for - FIGI: " << figi << ", TICKER: " << ticker << endl;
                     cout << "Unsubscription text:" << endl << subscriptionText << endl << "--------" << endl << endl;
                     webSocket.sendTextMessage( subscriptionText );

                     CASE_BREAK();
                 }

            CASE_STATE(stateUnsubscribeInstrumentInfo)
                 {
                     QString figi = figiIt->first, ticker = figiIt->second;
                     QString subscriptionText = pOpenApi->getStreamingApiInstrumentInfoUnsubscribeJson( figi );
                
                     cout << "/// Try to Unsubscribe to instrument info for - FIGI: " << figi << ", TICKER: " << ticker << endl;
                     cout << "Unsubscription text:" << endl << subscriptionText << endl << "--------" << endl << endl;
                     webSocket.sendTextMessage( subscriptionText );

                     CASE_BREAK();
                 }

            default:
                     if (jobDone.load()==false)
                     {
                         stopTimer.restart();
                         jobDone.store( true, std::memory_order_seq_cst  );
                     }

                     break;
        
        };


        if (jobDone.load()==true && stopTimer.elapsed()>stopTimeout)
        {
            cout << "--- Exiting due stop timeout" << endl;
            break;
        }

    }

    if (fConnected.load()!=false)
    {

       cout << "WebSocket forced closing" << endl;

       webSocket.close();

       while(fConnected.load()!=false)
       {
           QTest::qWait(1);
       }

       cout << "WebSocket closed" << endl;

    }

/*
    connect(&m_webSocket, &QWebSocket::connected, this, &EchoClient::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &EchoClient::closed);

    connect(&m_webSocket, &QWebSocket::textMessageReceived,
            this, &EchoClient::onTextMessageReceived);
*/
    
    return 0;
}



