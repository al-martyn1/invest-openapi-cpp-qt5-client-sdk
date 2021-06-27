/*! \file
    \brief 
 */

//NOTE: Umba headers must be first, at least "umba/umba.h"
#include "umba/umba.h"
#include "umba/simple_formatter.h"


#include <iostream>
#include <ostream>
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
#include "invest_openapi/streaming_helpers.h"

#include "invest_openapi/market_glass.h"
#include "invest_openapi/market_instrument_state.h"

#include "invest_openapi/operation_helpers.h"
#include "invest_openapi/order_helpers.h"

#include "invest_openapi/format_helpers.h"
#include "invest_openapi/terminal_helpers.h"

#include "invest_openapi/trading_terminal.h"

#include "invest_openapi/terminal_config.h"







INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("_tkf_simple_xterm");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cerr;
    using std::cout;
    using std::endl;


    namespace tkf=invest_openapi;
    using tkf::config_helpers::lookupForConfigFile;
    using tkf::config_helpers::FileReadable;

    QStringList lookupConfSubfolders = QString("conf;config").split( ';', Qt::SkipEmptyParts );

    auto logConfigFullFileName     = lookupForConfigFile( "logging.properties" , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto apiConfigFullFileName     = lookupForConfigFile( "config.properties"  , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto authConfigFullFileName    = lookupForConfigFile( "auth.properties"    , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto dbConfigFullFileName      = lookupForConfigFile( "database.properties", lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto instrumentsConfigFullFileName = lookupForConfigFile( "instruments.properties" , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto terminalConfigFullFileName = lookupForConfigFile( "terminal.properties" , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );



    auto apiConfig     = tkf::ApiConfig      ( apiConfigFullFileName  );
    auto authConfig    = tkf::AuthConfig     ( authConfigFullFileName );
    auto termConfig    = tkf::TerminalConfig ( terminalConfigFullFileName );



    QSharedPointer<tkf::DatabaseConfig> pDatabaseConfig = QSharedPointer<tkf::DatabaseConfig>( new tkf::DatabaseConfig(dbConfigFullFileName, tkf::DatabasePlacementStrategyDefault()) );

    QSharedPointer<tkf::LoggingConfig>  pLoggingConfig  = QSharedPointer<tkf::LoggingConfig> ( new tkf::LoggingConfig(logConfigFullFileName) );

    QSharedPointer<tkf::TerminalConfig> pTermConfig     = QSharedPointer<tkf::TerminalConfig>( new tkf::TerminalConfig(termConfig) );

    pLoggingConfig->debugSqlQueries = false;

    auto loggingConfig = *pLoggingConfig;



    auto dataLogFullFilename = pLoggingConfig->getDataLogFullName( logConfigFullFileName, "", "test.dat" );



    QSharedPointer<QSqlDatabase> pMainSqlDb = QSharedPointer<QSqlDatabase>( new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE")) );
    pMainSqlDb->setDatabaseName( pDatabaseConfig->dbMainFilename );

    if (!pMainSqlDb->open())
    {
      //qDebug() 
      cerr << pMainSqlDb->lastError().text() << endl;
      return 0;
    }



    QSharedPointer<tkf::IDatabaseManager> pMainDbMan = tkf::createMainDatabaseManager( pMainSqlDb, pDatabaseConfig, pLoggingConfig );

    pMainDbMan->applyDefDecimalFormatFromConfig( *pDatabaseConfig );



    QSharedPointer<tkf::IOpenApi> pOpenApi = tkf::createOpenApi( apiConfig, authConfig, loggingConfig );

    authConfig.setDefaultBrokerAccountForOpenApi(pOpenApi);


    tkf::DatabaseDictionaries dicts = tkf::DatabaseDictionaries(pMainDbMan);


    const QString operationsMaxAge = "10YEAR";

    // Custom code goes here

    //------------------------------

    tkf::trading_terminal::TradingTerminalData    terminalData( &dicts );

    terminalData.setConfig(pTermConfig);

    {
        QSettings settings(instrumentsConfigFullFileName, QSettings::IniFormat);
        QStringList instrumentList = settings.value("instruments" ).toStringList();
        terminalData.setInstrumentList(instrumentList);
    }



    std::map< QString, QSharedPointer< tkf::OpenApiCompletableFuture< tkf::OperationsResponse > > > awaitingOperationResponses;
    QSharedPointer< tkf::OpenApiCompletableFuture<tkf::OrdersResponse> >                            ordersResponse = 0;

    //------------------------------






    //------------------------------
    QWebSocket webSocket;
    console_helpers::SimpleHandleCtrlC ctrlC; // ctrlC.isBreaked()


    std::atomic<bool> fConnected = false;

    



    //termConfig

    auto printFigiInfoLine = [&]( QString figi )
                             {
                                 
                                 
                                 // cout << tkf::format_field( 0 /* leftSpace */ , 2 /* rightSpace */ , 12 /* fieldWidth */ , -1, figi );

                                 figi = dicts.findFigiByAnyIdString(figi);

                                 /*
                                 std::map< QString, tkf::trading_terminal::InstrumentInfoLineData >::const_iterator tdIt = terminalData.find(figi);
                                 if (tdIt == terminalData.end())
                                     return;

                                 std::vector< tkf::FieldFormat >::const_iterator fit = termConfig.fieldsFormat.begin();
                                 for(; fit != termConfig.fieldsFormat.end(); ++fit)
                                 {
                                     cout << tdIt->second.format_field( *fit );
                                 }
                                 */

                             };



    auto updateScreen =      [&]( )
                             {
                                 tkf::termClearScreen( cout /* , unsigned numLines = 50 */ );

                                 cout << "[" << terminalData.getStatusDateTime() << "] " << terminalData.getStatus() << endl;

                                 cout << endl;

                                 
                                 for( std::size_t nCol=0; nCol!=terminalData.getMainViewColsCount(); ++nCol)
                                 {
                                     cout << terminalData.formatMainViewColCaption(nCol); // << endl;
                                 }
                                 cout << endl;



                                 int nFigis = terminalData.getFigiCount();
                                 int nFigi  = 0;
                                 for(; nFigi<nFigis; ++nFigi)
                                 {
                                     auto figi = terminalData.getFigiByIndex(nFigi);
                                     printFigiInfoLine( figi );
                                     cout << endl;
                                 }

                                 cout << endl;

                                 terminalData.clearChangedFlags();

                             };



    auto updateFigiScreen =  [&]( QString figi )
                             {
                                 // Do something for FIGI

                                 updateScreen();

                             };


    auto updateStatusStr  =  [&]()
                             {
                                 updateScreen();
                             };



    auto onConnected = [&]()
            {
            
                fConnected.store( true, std::memory_order_seq_cst  );

                //std::map< QString, std::vector<tkf::OrderParams> >::const_iterator it = figiOrders.begin();
                
                //for( auto figi : instrumentList )
                auto it = terminalData.instrumentListBegin();
                for(; it!=terminalData.instrumentListEnd(); ++it)
                {
                    QString figi = *it;
                    figi = dicts.findFigiByAnyIdString(figi);

                    QString orderBookSubscriptionText         = pOpenApi->getStreamingApiOrderbookSubscribeJson( figi );
                    QString instrumentInfoSubscriptionText    = pOpenApi->getStreamingApiInstrumentInfoSubscribeJson( figi );
                   
                    QTest::qWait(5);
                    webSocket.sendTextMessage( instrumentInfoSubscriptionText );
                   
                    QTest::qWait(5);
                    webSocket.sendTextMessage( orderBookSubscriptionText );
                }

                terminalData.setStatus("Connected");

                // updateScreen();
                updateStatusStr();

            };

    auto onDisconnected = [&]()
            {
                fConnected.store( false, std::memory_order_seq_cst  );

                // Try to reconnect

                webSocket.open( pOpenApi->getStreamingApiNetworkRequest() );

                terminalData.setStatus("Disconnected");

                updateStatusStr();
            };




    auto onMessage = [&]( QString msg )
            {

                tkf::GenericStreamingResponse response;
                response.fromJson(msg);

                auto eventName = response.getEvent();

                if (eventName=="error")
                {
                    tkf::StreamingError streamingError;
                    streamingError.fromJson(msg);

                    //statusStr = QString("Streaming error: ") + streamingError.getPayload().getMessage();
                    terminalData.setStatus(QString("Streaming error: ") + streamingError.getPayload().getMessage());

                    updateStatusStr();
                }

                else if (eventName=="orderbook")
                {
                    tkf::StreamingOrderbookResponse response;
                    response.fromJson(msg);

                    tkf::MarketGlass marketGlass = tkf::MarketGlass::fromStreamingOrderbookResponse(response);

                    terminalData.update( marketGlass.figi, marketGlass );

                    //instrumentGlasses[marketGlass.figi] = marketGlass;

                    //tkf::trading_terminal::InstrumentInfoLineData::updateTerminalData(terminalData, dicts, marketGlass.figi, marketGlass);

                    updateFigiScreen(marketGlass.figi);
                }

                else if (eventName=="instrument_info")
                {
                    tkf::StreamingInstrumentInfoResponse response;
                    response.fromJson(msg);

                    tkf::MarketInstrumentState instrState = tkf::MarketInstrumentState::fromStreamingInstrumentInfoResponse( response );

                    // tkf::trading_terminal::InstrumentInfoLineData::updateTerminalData(terminalData, dicts, instrState.figi, instrState);

                    // instrumentStates[instrState.figi] = instrState;

                    terminalData.update( instrState.figi, instrState );

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



    

    auto checkAwaitingOperationResponses = [&]()
                                           {
                                               std::map< QString, std::vector< tkf::Operation > > completedOperationsByFigi;

                                               QString newStatusStr;
                                               // false on first error
                                               bool res = tkf::processAwaitingOperationResponses( pOpenApi, operationsMaxAge
                                                                                                , awaitingOperationResponses
                                                                                                , completedOperationsByFigi
                                                                                                , newStatusStr
                                                                                                );


                                               std::map< QString, std::vector< tkf::Operation > >::const_iterator it = completedOperationsByFigi.begin();
                                               for( ; it != completedOperationsByFigi.end(); ++it )
                                               {
                                                   terminalData.update( it->first, it->second );
                                               }

                                               /*
                                               if (tkf::mergeOperationMaps(instrumentOperations, completedOperationsByFigi))
                                               {
                                                   // если состав и/или количество элементов хотя бы в одном эелементе mergeTo изменилось
                                                   // Надо запросить состояние ордеров
                                                   // !!! Надо запросить портфолио
                                                   ordersResponse = pOpenApi->orders();
                                               }

                                               it = completedOperationsByFigi.begin();
                                               for( ; it != completedOperationsByFigi.end(); ++it )
                                               {
                                                   updateFigiScreen(it->first);
                                               }

                                               if (statusStr!=newStatusStr)
                                               {
                                                  statusStr = newStatusStr;
                                                  updateStatusStr();
                                               }
                                               */

                                               updateScreen();

                                           };



    auto requestForInstrumentOperations = [&]( QString figi )
                                          {
                                              figi = dicts.findFigiByAnyIdString(figi);
                                              if (awaitingOperationResponses.find(figi)!=awaitingOperationResponses.end())
                                                  return; // Already in queue
                                              auto operationsResponse   = pOpenApi->operations( operationsMaxAge, figi);
                                              awaitingOperationResponses[figi] = operationsResponse;
                                          };
                                           
    //
    

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

        Теоретически, если запрос по операциям вернул ошибку - надо бы перезапросить.


        По ордерам: надо ли постоянно запрашивать?

    
     */

    QElapsedTimer operationsRequestTimer;
    operationsRequestTimer.start();

    std::size_t requestCounter = 0;

    QStringList::const_iterator instrumentForOperationsIt = terminalData.instrumentListBegin(); // instrumentList.begin();

    for(; instrumentForOperationsIt!=terminalData.instrumentListEnd(); ++instrumentForOperationsIt, ++requestCounter)
    {
        tkf::checkWaitOnRequestsLimit( operationsRequestTimer, requestCounter );

        QString requestForFigi     = dicts.findFigiByAnyIdString(*instrumentForOperationsIt);
        QString requestForTicker   = dicts.getTickerByFigiChecked(requestForFigi);

        requestForInstrumentOperations(requestForFigi);

        checkAwaitingOperationResponses();

    }

    while(!awaitingOperationResponses.empty())
        checkAwaitingOperationResponses();




    //cout << "# Connecting Streaming API Web socket" << endl;
    tkf::connectStreamingWebSocket( pOpenApi, webSocket, onConnected, onDisconnected, onMessage );



    while( !ctrlC.isBreaked() )
    {
        QTest::qWait(1);

        if (ordersResponse && ordersResponse->isFinished())
        {
            std::map< QString, std::vector<OpenAPI::Order> >  activeOrders;

            if (!tkf::processCompletedOrdersResponse( ordersResponse, activeOrders ) )
            {
                ordersResponse = pOpenApi->orders(); // rerequest
            }
            else
            {
                ordersResponse = 0;
                //activeOrders.swap(activeOrdersTmp);

                std::map< QString, std::vector<OpenAPI::Order> >::const_iterator it = activeOrders.begin();
                for( ; it!=activeOrders.end(); ++it)
                {
                    terminalData.update( it->first, it->second );
                }

                /// !!! Нужно сравнить ордера по фигам, и найти те фиги, где поменялось
                updateScreen();
            }

        }

    }


    

    return 0;

    
}



