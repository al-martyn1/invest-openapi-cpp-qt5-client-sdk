/*! \file
    \brief Configs lookup test

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
    QCoreApplication::setApplicationName("test026");
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


    QWebSocket webSocket;
    console_helpers::SimpleHandleCtrlC ctrlC; // ctrlC.isBreaked()


    std::vector< std::pair< QString,QString > > figis = { { "BBG000BN56Q9" , "DSKY" }
                                                        , { "BBG000FWGSZ5" , "IRKT" }
                                                        , { "BBG000NLCCM3" , "LSNGP" }
                                                        , { "BBG000QQPXZ5" , "LNTA" }
                                                        , { "BBG000RMWQD4" , "ENPG" }
                                                        , { "BBG00178PGX3" , "MAIL" }
                                                        , { "BBG004730N88" , "SBER" }
                                                        , { "BBG004730RP0" , "GAZP" }
                                                        , { "BBG004731354" , "ROSN" }
                                                        , { "BBG0047315Y7" , "SBERP" }
                                                        , { "BBG00475KKY8" , "NVTK" }
                                                        , { "BBG004PYF2N3" , "POLY" }
                                                        , { "BBG004S683W7" , "AFLT" }
                                                        , { "BBG004S684M6" , "SIBN" }
                                                        , { "BBG005D1WCQ1" , "QIWI" }
                                                        , { "BBG006L8G4H1" , "YNDX" }
                                                        , { "BBG00B8NN386" , "GRNT" }
                                                        , { "BBG00JXPFBN0" , "FIVE" }
                                                        , { "BBG00VPKLPX4" , "POGR" }
                                                        , { "BBG00Y91R9T3" , "OZON" }
                                                        , { "BBG0013HGFT4" , "USD000UTSTOM" }
                                                        };

    // https://habr.com/ru/post/517918/

    // Запарюсь что ли с атомиками
    // volatile bool connected = false;

    std::atomic<bool> fConnected = false;

    auto onConnected = [&]()
            {
                using std::cout;
                using std::endl;
            
                cout << "*** Streaming API Web socket connected" << endl;

                // cout << "Sending test subscription to ROSN/BBG004731354 orderbook" << endl;

                // BBG004731354 - ROSN
                // QString subscriptionText = pOpenApi->getStreamingApiOrderbookJsonSentenceSubscribe( "BBG004731354" );

                // BBG0047315Y7 - SBERP
                // BBG004730N88 - SBER

                // cout << "Subscription text:" << endl;
                // cout << subscriptionText << endl;
                // webSocket.sendTextMessage( subscriptionText );

                // connected = true;

                fConnected.store( true, std::memory_order_seq_cst  );

            };

    auto onDisconnected = [&]()
            {
                using std::cout;
                using std::endl;
            
                cout << "*** Streaming API Web socket disconnected" << endl;
            };

    auto onMessage = [&]( QString msg )
            {
                using std::cout;
                using std::endl;
            
                cout << "*** Streaming API Web socket received message: " << msg << endl;
            };


    // https://wiki.qt.io/New_Signal_Slot_Syntax

    webSocket.connect( &webSocket, &QWebSocket::connected             , onConnected    );
    webSocket.connect( &webSocket, &QWebSocket::disconnected          , onDisconnected );

    webSocket.connect( &webSocket, &QWebSocket::textMessageReceived   , onMessage      );

    webSocket.open( pOpenApi->getStreamingApiNetworkRequest() );

    cout << "Press Ctrl+C to break process" << endl;

    std::vector< QString >::const_iterator figiIt  = figis.begin();
    std::vector< QString >::const_iterator figiEnd = figis.end  ();

    QElapsedTimer stopTimer   ;  stopTimer   .start();
    QElapsedTimer requestTimer;  requestTimer.start();

    while(!ctrlC.isBreaked())
    {
        QTest::qWait(1);

        if (requestTimer.elapsed()>1000)
        {
            if (figiIt!=figiEnd)
            {
                QString figi             = figiIt->first ;
                QString tiker            = figiIt->second;
                QString subscriptionText = pOpenApi->getStreamingApiOrderbookJsonSentenceSubscribe( figi );

                cout << "Try to subscribe to - FIGI: " << figi << ", TICKER: " << ticker << endl;
                cout << "Subscription text:" << endl;
                cout << subscriptionText << endl << "--------" << endl << endl;

            }
        }

    }
    

/*
    connect(&m_webSocket, &QWebSocket::connected, this, &EchoClient::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &EchoClient::closed);

    connect(&m_webSocket, &QWebSocket::textMessageReceived,
            this, &EchoClient::onTextMessageReceived);
*/
    
    return 0;
}



