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

#include "invest_openapi/format_helpers.h"




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


    QStringList instrumentList;
    {
        QSettings settings(instrumentsConfigFullFileName, QSettings::IniFormat);
        instrumentList = settings.value("instruments" ).toStringList();
    }


    tkf::DatabaseDictionaries dicts = tkf::DatabaseDictionaries(pMainDbMan);


    QStringList instrumentList;
    {
        QSettings settings(instrumentsConfigFullFileName, QSettings::IniFormat);
        instrumentList = settings.value("instruments" ).toStringList();
    }



    // Custom code goes here

    std::map< QString, tkf::MarketInstrumentState >   instrumentStates;
    std::map< QString, tkf::MarketGlass           >   instrumentGlasses;


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



    auto onConnected = [&]()
            {
                using std::cout;  using std::endl;
            
                fConnected.store( true, std::memory_order_seq_cst  );

                //std::map< QString, std::vector<tkf::OrderParams> >::const_iterator it = figiOrders.begin();
                for( auto figi : instrumentList)
                {
                    QString figi = dicts.findFigiByAnyIdString(figi);

                    QString orderBookSubscriptionText         = pOpenApi->getStreamingApiOrderbookSubscribeJson( figi );
                    QString instrumentInfoSubscriptionText    = pOpenApi->getStreamingApiInstrumentInfoSubscribeJson( figi );
                   
                    QTest::qWait(5);
                    webSocket.sendTextMessage( instrumentInfoSubscriptionText );
                   
                    QTest::qWait(5);
                    webSocket.sendTextMessage( orderBookSubscriptionText );
                }


                statusStr = "Connected";

            };

    auto onDisconnected = [&]()
            {
                using std::cout;  using std::endl;
            
                fConnected.store( false, std::memory_order_seq_cst  );

                // Try to reconnect

                webSocket.open( pOpenApi->getStreamingApiNetworkRequest() );

                statusStr = "Disconnected";


                //UNDONE: !!! Update screen
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

                    //cout << "# !!! Streaming error: " << streamingError.getPayload().getMessage() << endl;
                    statusStr = QString("Streaming error: ") + streamingError.getPayload().getMessage();
                    //UNDONE: !!! Update screen
                }

                else if (eventName=="orderbook")
                {
                    tkf::StreamingOrderbookResponse response;
                    response.fromJson(msg);

                    tkf::MarketGlass marketGlass = tkf::MarketGlass::fromStreamingOrderbookResponse(response);

                    //if (marketGlass.isValid() && marketGlass.figi==orderParams.figi)
                    //    instrumentGlass = marketGlass;

                    //UNDONE: !!! Update screen
                }

                else if (eventName=="instrument_info")
                {
                    tkf::StreamingInstrumentInfoResponse response;
                    response.fromJson(msg);

                    tkf::MarketInstrumentState instrState = tkf::MarketInstrumentState::fromStreamingInstrumentInfoResponse( response );

                    if (instrState.isValid() && instrState.figi==orderParams.figi)
                    {
                        //instrumentState = instrState;
                        //cout << "# !!! Streaming error: got an invalid MarketInstrumentState - " << msg << endl;
                    }

                    //UNDONE: !!! Update screen
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


    //cout << "# Connecting Streaming API Web socket" << endl;
    webSocket.connect( &webSocket, &QWebSocket::connected             , onConnected    );
    webSocket.connect( &webSocket, &QWebSocket::disconnected          , onDisconnected );
    webSocket.connect( &webSocket, &QWebSocket::textMessageReceived   , onMessage      );

    webSocket.open( pOpenApi->getStreamingApiNetworkRequest() );


    while(!ctrlC.isBreaked() )
    {
        QTest::qWait(1);
    }


    
    return 0;
}



