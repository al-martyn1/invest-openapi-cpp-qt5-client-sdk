/*! \file
    \brief Тест Тинькофф Streaming API - подписка на несколько десятков стаканов

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

    qDebug().nospace().noquote()<<"";
    qDebug().nospace().noquote()<<"";

    QWebSocket webSocket;
    console_helpers::SimpleHandleCtrlC ctrlC; // ctrlC.isBreaked()


    typedef std::pair< QString,QString >  figi_info_pair_t;

    std::vector< figi_info_pair_t > figis = { { "BBG000BN56Q9" ,   "DSKY"           }
                                            , { "BBG000FWGSZ5" ,   "IRKT"           }
                                            , { "BBG000NLCCM3" ,   "LSNGP"          }
                                            , { "BBG000QQPXZ5" ,   "LNTA"           }
                                            , { "BBG000RMWQD4" ,   "ENPG"           }
                                            , { "BBG00178PGX3" ,   "MAIL"           }
                                            , { "BBG004730N88" ,   "SBER"           }
                                            , { "BBG004730RP0" ,   "GAZP"           }
                                            , { "BBG004731354" ,   "ROSN"           }
                                            , { "BBG0047315Y7" ,   "SBERP"          }
                                            , { "BBG00475KKY8" ,   "NVTK"           }
                                            , { "BBG004PYF2N3" ,   "POLY"           }
                                            , { "BBG004S683W7" ,   "AFLT"           }
                                            , { "BBG004S684M6" ,   "SIBN"           }
                                            , { "BBG005D1WCQ1" ,   "QIWI"           }
                                            , { "BBG006L8G4H1" ,   "YNDX"           }
                                            , { "BBG00B8NN386" ,   "GRNT"           }
                                            , { "BBG00JXPFBN0" ,   "FIVE"           }
                                            , { "BBG00VPKLPX4" ,   "POGR"           }
                                            , { "BBG00Y91R9T3" ,   "OZON"           }
                                            , { "BBG0013HGFT4" ,   "USD000UTSTOM"   }
                                            , { "BBG000HLJ7M4" ,   "IDCC"           }
                                            , { "BBG002293PJ4" ,   "RH"             }
                                            , { "BBG000BPL8G3" ,   "MTSC"           }
                                            , { "BBG000CTQBF3" ,   "SBUX"           }
                                            , { "BBG000BH0FR6" ,   "SGEN"           }
                                            , { "BBG004S68758" ,   "BANE"           }
                                            , { "BBG004MN1R41" ,   "NAVI"           }
                                            , { "BBG000BPNP00" ,   "MXIM"           }
                                            , { "BBG000BWS3F3" ,   "WGO"            }
                                            , { "BBG000BF0K17" ,   "CAT"            }
                                            , { "BBG00172J7S9" ,   "OKEY"           }
                                            , { "BBG000BTGKK9" ,   "TREX"           }
                                            , { "BBG000CZ4KF3" ,   "STMP"           }
                                            , { "BBG000BZJQL8" ,   "IART"           }
                                            , { "BBG000BS9HN3" ,   "ROG"            }
                                            , { "BBG000BS5DR2" ,   "RHI"            }
                                            , { "BBG000BDHD29" ,   "UNVR"           }
                                            , { "BBG0077VNXV6" ,   "PYPL"           }
                                            , { "BBG001R72SR9" ,   "UI"             }
                                            , { "BBG001JZPSQ2" ,   "SONO"           }
                                            , { "BBG000FV1Z23" ,   "CCI"            }
                                            , { "BBG000BFDLV8" ,   "CERN"           }
                                            , { "BBG000BGXZB5" ,   "AEO"            }
                                            , { "BBG002WMH2F2" ,   "TMX"            }
                                            , { "BBG000BMWKC5" ,   "KMT"            }
                                            , { "BBG000DZJVH0" ,   "NXST"           }
                                            , { "BBG000BQHGR6" ,   "OKE"            }
                                            , { "BBG000BNSZP1" ,   "MCD"            }
                                            , { "BBG000BHJSC4" ,   "BBSI"           }
                                            , { "BBG000BGRY34" ,   "CVS"            }
                                            , { "BBG000VMWHH5" ,   "DISCK"          }
                                            , { "BBG000BKH263" ,   "NEO"            }
                                            , { "BBG000QW7VC1" ,   "HCA"            }
                                            , { "BBG005YHY0Q7" ,   "JD"             }
                                            , { "BBG000C6GN04" ,   "QDEL"           }
                                            , { "BBG00FH3FM15" ,   "TCRR"           }
                                            , { "BBG000NLCCM3" ,   "LSNGP"          } // дубль, посмотрим
                                            , { "BBG000C41023" ,   "UDR"            }
                                            };

    // https://habr.com/ru/post/517918/

    // Запарюсь что ли с атомиками
    // volatile bool connected = false;

    std::atomic<bool> fConnected = false;
    std::atomic<bool> jobDone    = false;

    auto onConnected = [&]()
            {
                using std::cout;
                using std::endl;
            
                fConnected.store( true, std::memory_order_seq_cst  );

                cout << "*** Streaming API Web socket connected" << endl;
                cout << endl;


                QString subscriptionText = pOpenApi->getStreamingApiOrderbookSubscribeJson( "BBG004731354", -1 );
                
                cout << "!!! Try to subscribe to - FIGI: " << "BBG004731354" << ", TICKER: " << "ROSN" << endl;
                cout << "Subscription text:" << endl;
                cout << subscriptionText << endl << "--------" << endl << endl;
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
            
                cout << "*** Streaming API Web socket received message: " << endl<< msg << endl << "--------" << endl << endl;
                
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

    const std::uint64_t requestAdditionDelta = 3000; // ms
    const std::uint64_t stopTimeout = figis.size()*requestAdditionDelta + 60*1000; // Ждём минуту после того, как закончатся фиги для добавления, и выходим 

    enum State
    {
        stateRequestFigis,
        stateFinal

    };


    State state = stateRequestFigis;


    while(!ctrlC.isBreaked())
    {
        QTest::qWait(1);

        if (requestTimer.elapsed() > requestAdditionDelta) // Добавление реквестов
        {

            // Думаю попробовать добавление разнородных запросов

            switch(state)
            {
                case stateRequestFigis:
                     if (figiIt==figiEnd)
                     {
                         state = (State)(state+1);
                         requestTimer.restart();

                         //NOTE: no break here - we immediately go to the next case
                     }
                     else
                     {
                         QString figi             = figiIt->first ;
                         QString ticker           = figiIt->second;
                         QString subscriptionText = pOpenApi->getStreamingApiOrderbookSubscribeJson( figi );
                    
                         cout << "!!! Try to subscribe to - FIGI: " << figi << ", TICKER: " << ticker << endl;
                         cout << "Subscription text:" << endl;
                         cout << subscriptionText << endl << "--------" << endl << endl;
                         webSocket.sendTextMessage( subscriptionText );

                         ++figiIt;
                         requestTimer.restart();


                         stopTimer.restart();

                         break;
                     }
                     

                default:
                         jobDone.store( true, std::memory_order_seq_cst  );
                         break;

            
            };

        } // if (requestTimer.elapsed()>1000)


        if (stopTimer.elapsed() > stopTimeout)
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



