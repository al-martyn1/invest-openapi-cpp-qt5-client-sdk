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


    auto onConnected = [&]()
            {
                using std::cout;
                using std::endl;
            
                cout << "Streaming API Web socket connected" << endl;

                cout << "Sending test subscription to ROSN/BBG004731354 orderbook" << endl;

                // BBG004731354 - ROSN
                QString subscriptionText = pOpenApi->getStreamingApiOrderbookJsonSentenceSubscribe( "BBG004731354" );

                // BBG0047315Y7 - SBERP
                // BBG004730N88 - SBER

                cout << "Subscription text:" << endl;
                cout << subscriptionText << endl;
                webSocket.sendTextMessage( subscriptionText );

            };

    auto onDisconnected = [&]()
            {
                using std::cout;
                using std::endl;
            
                cout << "Streaming API Web socket disconnected" << endl;
            };

    auto onMessage = [&]( QString msg )
            {
                using std::cout;
                using std::endl;
            
                cout << "Streaming API Web socket received message: " << msg << endl;
            };


    // https://wiki.qt.io/New_Signal_Slot_Syntax

    webSocket.connect( &webSocket, &QWebSocket::connected             , onConnected    );
    webSocket.connect( &webSocket, &QWebSocket::disconnected          , onDisconnected );

    webSocket.connect( &webSocket, &QWebSocket::textMessageReceived   , onMessage      );

    webSocket.open( pOpenApi->getStreamingApiNetworkRequest() );

    cout << "Press Ctrl+C to break process" << endl;


    while(!ctrlC.isBreaked())
    {
        QTest::qWait(1);
    }
    

/*
    connect(&m_webSocket, &QWebSocket::connected, this, &EchoClient::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &EchoClient::closed);

    connect(&m_webSocket, &QWebSocket::textMessageReceived,
            this, &EchoClient::onTextMessageReceived);
*/
    
    return 0;
}



