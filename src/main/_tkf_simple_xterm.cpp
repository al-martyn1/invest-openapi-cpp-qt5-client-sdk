/*! \file
    \brief 
 */

//NOTE: Umba headers must be first, at least "umba/umba.h"
#include "umba/umba.h"
#include "umba/simple_formatter.h"
#include "umba/char_writers.h"


#include <iostream>
#include <ostream>
#include <sstream>
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
#include "invest_openapi/portfolio_helpers.h"

#include "invest_openapi/order_params.h"

#include "invest_openapi/terminal_helpers.h"
#include "invest_openapi/format_helpers.h"

#include "invest_openapi/terminal_config.h"
#include "invest_openapi/terminal_input.h"
#include "invest_openapi/trading_terminal.h"

#include "invest_openapi/placed_order_info.h"

#include "invest_openapi/logging.h"


umba::StdStreamCharWriter      coutWriter(std::cout);
umba::StdStreamCharWriter      cerrWriter(std::cerr);
umba::NulCharWriter            nulWriter;

umba::SimpleFormatter          tout(&coutWriter); // terminal out - like cout




INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("_tkf_simple_xterm");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    // using std::cerr;
    // using std::cout;
    // using std::endl;

    using namespace umba::omanip;


    namespace tkf=invest_openapi;
    namespace trd=invest_openapi::trading_terminal;

 
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


    std::map<QString, QString> logHandlers = pLoggingConfig->getLogHandlers( logConfigFullFileName );

    tkf::initLogging( logHandlers
                    , pLoggingConfig->logDateTimeFormat
                    , pLoggingConfig->logLevel
                    , pLoggingConfig->logContext
                    );

    qDebug() << "Logging initialized";

    auto dataLogFullFilename = pLoggingConfig->getDataLogFullName( logConfigFullFileName, "", "test.dat" );



    QSharedPointer<QSqlDatabase> pMainSqlDb = QSharedPointer<QSqlDatabase>( new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE")) );
    pMainSqlDb->setDatabaseName( pDatabaseConfig->dbMainFilename );

    if (!pMainSqlDb->open())
    {
      //qDebug() 
      // cerr << pMainSqlDb->lastError().text() << endl;
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


    // terminalData полностью инициализированна


    std::map< QString, QSharedPointer< tkf::OpenApiCompletableFuture< tkf::OperationsResponse > > > awaitingOperationResponses;

    QSharedPointer< tkf::OpenApiCompletableFuture<tkf::OrdersResponse> >                            ordersResponse = 0;
    QDateTime                                                                                       lastOrdersResponseLocalDateTime = QDateTime::currentDateTime();

    // Сразу запросим, чего откладывать в долгий ящик
    QSharedPointer< tkf::OpenApiCompletableFuture<tkf::PortfolioResponse> >                         portfolioResponse = pOpenApi->portfolio();
    QDateTime                                                                                       lastPortfolioResponseLocalDateTime = QDateTime::currentDateTime();
    
    QSharedPointer< tkf::OpenApiCompletableFuture<tkf::PortfolioCurrenciesResponse> >               portfolioCurrenciesResponse = pOpenApi->portfolioCurrencies();
    QDateTime                                                                                       lastPortfolioCurrenciesResponseLocalDateTime = QDateTime::currentDateTime();


    std::map< std::string, tkf::OrderRequestData >     activeOrderRequests;
    

    tkf::SimpleTerminalLineEditImplBase  *pTerminalInputEdit = 0;

    //------------------------------



    auto normalColor        = UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::white , umba::term::colors::black, false, false, false );
    auto statusColor        = normalColor; // UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::white , umba::term::colors::black, false, false, false );
    auto captionColor       = normalColor; // UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::yellow, umba::term::colors::black, false, false, false );
    auto editorColor        = normalColor; //UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::cyan  , umba::term::colors::black, false, false, false );
    auto editorHintColor    = UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::cyan  , umba::term::colors::black, false, false, false );
    auto editorCursorColor  = UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::white , umba::term::colors::black, false, false, false );
    // auto editorCursorColor  = UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::black , umba::term::colors::cyan , false, false, false );

    // UMBA_TERM_COLORS_MAKE_COMPOSITE( fgColor, bgColor, fBright, fInvert, fBlink )
    //umba::term::colors::white

    //------------------------------

    tout << term::caret(0);



    //------------------------------

    QWebSocket webSocket;
    console_helpers::SimpleHandleCtrlC    ctrlC; // ctrlC.isBreaked()
    tkf::SimpleTerminalInput              simpleInput;

    std::atomic<bool> fConnected = false;



    //------------------------------
    
    auto printEditorText = [&]( const std::string &text, const std::string &autocompleteHint )
                             {

                                 tout << term::move2abs0 ;


                                 // Ready to print connection state
                                 tout << term::move2down;
                                 tout << term::move2down;


                                 // Ready to print status
                                 tout << term::move2down;
                                 tout << term::move2down;


                                 // Ready to print balance
                                 tout << term::move2down;
                                 tout << term::move2down;


                                 // Ready to print table

                                 // Caption

                                 if (pTermConfig->hbreakCaptionBefore)
                                     tout << term::move2down;
                                 if (pTermConfig->hbreakCaptionAfter)
                                     tout << term::move2down;

                                 tout << term::move2down;

                                 // Body

                                 int nFigis = terminalData.getFigiCount();
                                 int nFigi  = 0;
                                 for(; nFigi<nFigis; ++nFigi)
                                 {
                                     if (pTermConfig->hbreakStyleRegular!=0 && nFigi && nFigi!=(nFigis-1) && (nFigi%pTermConfig->hbreakRegular)==0)
                                     {
                                         tout << term::move2down;
                                     }

                                     tout << term::move2down;
                                 }

                                 if (pTermConfig->hbreakTableAfter)
                                 {
                                     tout << term::move2down;
                                 }

                                 tout << term::move2down;

                                 tout << term::move2line0;
                                 tout << color(pTermConfig->colors.inputPrompt) << ">"; // Prompt
                                 tout << color(pTermConfig->colors.inputText  ) << text;
                                 
                                 if (!autocompleteHint.empty())
                                 {
                                     tout << color(pTermConfig->colors.inputHint  ) << autocompleteHint;
                                 }
                                 
                                 tout << color(editorCursorColor);
                                 
                                 tout << color(pTermConfig->colors.inputCaret ) << "_" << color(normalColor) << " " << term::clear(2);

                                 //pTermConfig->colors.

                             };


    auto printFigiInfoLine = [&]( QString figi )
                             {
                                 
                                 // cout << tkf::format_field( 0 /* leftSpace */ , 2 /* rightSpace */ , 12 /* fieldWidth */ , -1, figi );

                                 std::vector< std::size_t > mvColSizes = terminalData.getMainViewColSizes();

                                 std::size_t xPos = 0;

                                 for( std::size_t nCol=0; nCol!=terminalData.getMainViewColsCount(); xPos+=mvColSizes[nCol], ++nCol)
                                 {
                                     tout << term::move2lpos(xPos) << term::clear_line((int)mvColSizes[nCol]);

                                     auto cellClr  = umba::term::colors::white;
                                     auto cellText = terminalData.formatMainViewField( figi, nCol, &cellClr );
                                     tout << color(cellClr) << cellText
                                          << color(pTermConfig->colors.tableText);
                                 }

                             };



    auto updateScreen =      [&]( )
                             {
                                 // tkf::termClearScreen( cout /* , unsigned numLines = 50 */ );

                                 // tout << term::caret(0);

                                 tout << term::move2abs0 ;

                                 auto connectionStateColor = terminalData.getConnectionStateColor();

                                 // Ready to print connection state
                                 tout << "[" << terminalData.getConnectionStateDateTimeStr().toStdString() << "] " 
                                      << color(connectionStateColor)
                                      << terminalData.getConnectionStateStr().toStdString() // << endl;
                                      << color( pTermConfig->colors.getStateColor(2) )
                                      ;
                                 tout << term::clear(2);


                                 tout << term::move2down;
                                 tout << term::move2down;


                                 // Ready to print status
                                 tout << "[" << terminalData.getStatusDateTimeStr().toStdString() << "] " << terminalData.getStatus().toStdString(); // << endl;
                                 tout << term::clear(2);

                                 tout << term::move2down;
                                 tout << term::move2down;


                                 // Ready to print balance
                                 tout << terminalData.getCurrenciesStr().toStdString();
                                 tout << term::clear(2);

                                 tout << term::move2down;
                                 tout << term::move2down;


                                 // Ready to print table

                                 std::vector< std::size_t > mvColSizes = terminalData.getMainViewColSizes();

                                 std::size_t xPos = 0;

                                 std::size_t mainViewTotalWidth = terminalData.getMainViewTotalWidth();
                                 

                                 // Caption
                                 // tout << color()

                                 if (terminalData.isCaptionChanged())
                                 {
                                     if (pTermConfig->hbreakCaptionBefore)
                                     {
                                         tout << color(pTermConfig->colors.captionHBreakBefore);
                                         tout << pTermConfig->getHBreak("caption.before", mainViewTotalWidth);
                                         tout << color(pTermConfig->colors.tableText);
                                         tout << term::move2down;
                                     }

                                     for( std::size_t nCol=0; nCol!=terminalData.getMainViewColsCount(); xPos+=mvColSizes[nCol], ++nCol)
                                     {
                                         tout << term::move2lpos(xPos) << term::clear_line((int)mvColSizes[nCol]);
                                         tout << color(pTermConfig->colors.caption) << terminalData.formatMainViewColCaption(nCol); // << endl;
                                     }

                                     if (pTermConfig->hbreakCaptionAfter)
                                     {
                                         tout << term::move2down;
                                         tout << color(pTermConfig->colors.captionHBreakAfter );
                                         tout << pTermConfig->getHBreak("caption.after", mainViewTotalWidth);
                                         tout << color(pTermConfig->colors.tableText);
                                     }

                                 }
                                 else
                                 {
                                     if (pTermConfig->hbreakCaptionBefore)
                                         tout << term::move2down;
                                     if (pTermConfig->hbreakCaptionAfter)
                                         tout << term::move2down;
                                 }

                                 tout << term::move2down;

                                 // move2lpos(x)


                                 // Body

                                 int nFigis = terminalData.getFigiCount();
                                 int nFigi  = 0;
                                 for(; nFigi<nFigis; ++nFigi)
                                 {
                                     if (pTermConfig->hbreakStyleRegular!=0 && nFigi && nFigi!=(nFigis-1) && (nFigi%pTermConfig->hbreakRegular)==0)
                                     {
                                         if (terminalData.isCaptionChanged())
                                         {
                                             tout << color(pTermConfig->colors.tableRegularHBreak);
                                             tout << pTermConfig->getHBreak("regular", mainViewTotalWidth);
                                             tout << color(pTermConfig->colors.tableText);
                                         }
                                         tout << term::move2down;
                                     }

                                     auto figi = terminalData.getFigiByIndex(nFigi);
                                     if (terminalData.isFigiChanged(figi))
                                         printFigiInfoLine( figi );

                                     tout << term::move2down;
                                 }

                                 if (pTermConfig->hbreakTableAfter)
                                 {
                                     if (terminalData.isCaptionChanged())
                                     {
                                         tout << color(pTermConfig->colors.tableHBreakAfter);
                                         tout << pTermConfig->getHBreak("table.after", mainViewTotalWidth);
                                         tout << color(pTermConfig->colors.tableText);
                                     }
                                     tout << term::move2down;
                                 }

                                 tout << term::move2down;


                                 // tout << term::clear(-1); // Не надо ничего очищать


                                 // if (pTerminalInputEdit)
                                 //    pTerminalInputEdit->updateView(); // редактор, когда надо, сам себя обновляет
                                 //printEditorText(text);

                                 // tout << term::caret(1);

                                 terminalData.clearChangedFlags();

                             };



    auto updateFigiScreen =  [&]( QString figi ) { updateScreen(); };

    auto updateStatusStr  =  [&]() 
                             { 
                                 tout << term::move2abs0 ;


                                 // Ready to print connection state
                                 tout << term::move2down;
                                 tout << term::move2down;


                                 // Ready to print status
                                 tout << "[" << terminalData.getStatusDateTimeStr().toStdString() << "] " << terminalData.getStatus().toStdString(); // << endl;
                                 tout << term::clear(2);

                                 tout << term::move2down;
                                 tout << term::move2down;


                                 // Ready to print balance

                             };



    auto onEditTextModified = [&]( tkf::SimpleTerminalLineEditImplBase *pEdit, std::string &text )
                              {
                                  //printEditorText(text);

                                  if (text.empty())
                                      return;

                                  if (text[0]=='=')
                                  {
                                      // На клавиатуре минус '-' работает без шифта, с шифтом - даёт '_'
                                      // Плюс же, наоборот, получается с шифтом, без шифта - выдаётся '='
                                      // Довольно неудобно гонять шифт туда-сюда, тем более, что никакой нагрузки на символ '=' пока нет.
                                      // Поэтому - корректируем ввод
                                      // Текст нам даден по ссылке, и мы можем его модифицировать, как минимум - менять одни символы на другие
                                      // (покакрайней мере, так предполагалось во время реализации редактора строки, и вроде должно работать)

                                      text[0] = '+';
                                  }


                                  auto tmpText = tkf::prepareOrderParamsString( text );

                                  if (tmpText.empty())
                                      return;

                                  bool acltSet = false;

                                  if ( /* !text.empty() &&  */ text.back()==' ') // оригинальный текст не пустой, проверка добавлена
                                  {
                                      // В оригинальном тексте на последней позиции - пробел, значит никаких подсказок не нужно
                                      return;
                                  }

                                  if (tmpText[0]=='-' || tmpText[0]=='+')
                                  {
                                      // We got an 'place order' command

                                      std::vector<std::string> orderParamsStrVec = tkf::splitOrderParamsString( tmpText );

                                      if ( orderParamsStrVec.size()>1 ) // at least 2 and last - is id
                                      {
                                          auto idCandy = orderParamsStrVec.back();
                                          if ( tkf::prepareOrderIsIdString(idCandy) )
                                          {
                                              QString qIdCandy  = QString::fromStdString(idCandy);
                                              QString qFoundId   = terminalData.getTickerLikeThis( qIdCandy );
                                              if (!qFoundId.isEmpty() && qFoundId.size()>qIdCandy.size() )
                                              {
                                                  QString hintStr = qFoundId;
                                                  hintStr.remove( 0, qIdCandy.size() );
                                                  hintStr += " ";
                                                  pEdit->setAclt(hintStr.toStdString());
                                                  acltSet = true;
                                              }
                                          }
                                      }
                                  }

                                  if (!acltSet)
                                  {
                                      pEdit->clrAclt();
                                  }

                              };


    auto onEditTextComplete = [&](tkf::SimpleTerminalLineEditImplBase *pEdit, std::string &text ) -> bool
                              {
                                  auto canonicalOrderRequestString = tkf::mergeOrderParamsString(tkf::splitOrderParamsString( tkf::prepareOrderParams(text) ));

                                  tkf::OrderParams orderParams;

                                  int parsingRes = tkf::parseOrderParams( tkf::splitOrderParamsString( tkf::prepareOrderParams(canonicalOrderRequestString) )
                                                                        , dicts, orderParams
                                                                        );

                                  std::ostringstream oss;

                                  if (parsingRes<0)
                                  {
                                      oss << "Order Request: '" << canonicalOrderRequestString 
                                          <<"': Error: invalid or missing argument #" << -parsingRes; // << endl;
                                      terminalData.setStatus(QString::fromStdString(oss.str()));
                                      updateStatusStr();
                                      return true; // Keep string for further editing
                                  }


                                  if (pTermConfig->readOnlyMode)
                                  {
                                      oss << "Order Request: '" << canonicalOrderRequestString 
                                          <<"': OK, but not sent due 'terminal.read-only' option";
                                      terminalData.setStatus(QString::fromStdString(oss.str()));

                                      return false; // Commnd Ok, allow to clear input
                                  }



                                  tkf::MarketInstrumentState   instrumentState;
                                  tkf::MarketGlass             instrumentGlass;


                                  if ( !terminalData.getInstrumentMarketState(orderParams.figi, instrumentState) || !instrumentState.isTradeStatusNormalTrading() )
                                  {
                                      oss << "Order Request: '" << canonicalOrderRequestString 
                                          <<"': Error: instrument is not currently in trading"; // << endl;
                                      terminalData.setStatus(QString::fromStdString(oss.str()));
                                      updateStatusStr();
                                      return true; // Keep string for further editing
                                  }

                                  if ( instrumentState.lotSize == 0 )
                                  {
                                      oss << "Order Request: '" << canonicalOrderRequestString 
                                          <<"': Error: unknown instrument lot size"; // << endl;
                                      terminalData.setStatus(QString::fromStdString(oss.str()));
                                      updateStatusStr();
                                      return true; // Keep string for further editing
                                  }

                                  if ( instrumentState.priceIncrement==0 )
                                  {
                                      oss << "Order Request: '" << canonicalOrderRequestString 
                                          <<"': Error: unknown instrument price increment"; // << endl;
                                      terminalData.setStatus(QString::fromStdString(oss.str()));
                                      updateStatusStr();
                                      return true; // Keep string for further editing
                                  }

                                  if ( !terminalData.getInstrumentMarketGlass(orderParams.figi, instrumentGlass) )
                                  {
                                      oss << "Order Request: '" << canonicalOrderRequestString 
                                          <<"': Error: no instrument Bids/Asks info"; // << endl;
                                      terminalData.setStatus(QString::fromStdString(oss.str()));
                                      updateStatusStr();
                                      return true; // Keep string for further editing
                                  }


                                  unsigned numLots = orderParams.calcNumLots( instrumentState.lotSize );
                                 
                                  orderParams = orderParams.getAdjusted( instrumentGlass.getPriceSpreadPoints(instrumentState.priceIncrement)
                                                                       , instrumentState.priceIncrement
                                                                       , instrumentGlass.getAsksMinPrice()
                                                                       , instrumentGlass.getBidsMaxPrice()
                                                                       );
                                 
                                  if (!orderParams.isLimitPriceCorrect(instrumentGlass.getAsksMinPrice(), instrumentGlass.getBidsMaxPrice()))
                                  {
                                      oss << "Order Request: '" << canonicalOrderRequestString 
                                          <<"': Error: may be wrong price taken, or operation type was confused"; // << endl;
                                      terminalData.setStatus(QString::fromStdString(oss.str()));
                                      updateStatusStr();
                                      return true; // Keep string for further editing
                                  }


                                  // Тут начинаем веселье

                                  if (activeOrderRequests.find(canonicalOrderRequestString)!=activeOrderRequests.end())
                                  {
                                      oss << "Order Request: '" << canonicalOrderRequestString 
                                          <<"': Error: order is already in progress"; // << endl;
                                      terminalData.setStatus(QString::fromStdString(oss.str()));
                                      updateStatusStr();
                                      return true; // Keep string for further editing
                                  }

                                  //NOTE: !!! debugging
                                  // Temporary exit, for input debugging without real server calls
                                  // return false; // Allow new input


                                  // На самом деле досюда мы не доберёмся

                                  bool newInstrument = false;

                                  /*

                                  if (terminalData.isKnownInstrument(orderParams.figi))
                                  {
                                      if ( !terminalData.addInstrument( orderParams.figi ) )
                                      {
                                          newInstrument = true;

                                          // Нужно добавить инструмент в подписки на веб-сокете
                                      }
                                      
                                  }
                                  */


                                  tkf::OrderRequestData &newOrderRequestData = activeOrderRequests[ canonicalOrderRequestString ];

                                  newOrderRequestData.orderInputParams    = canonicalOrderRequestString;
                                  newOrderRequestData.orderAdjustedParams = orderParams.toString();
                                  newOrderRequestData.orderParams         = orderParams;
                                  
                                  newOrderRequestData.operationTimer.start();

                                  if (orderParams.isOrderTypeLimit())
                                  {
                                      newOrderRequestData.limitOrderResponse = pOpenApi->ordersLimitOrder( orderParams.figi
                                                                                                         , orderParams.getOpenApiOperationType()
                                                                                                         , numLots, orderParams.orderPrice
                                                                                                         );
                                  }
                                  else
                                  {
                                      newOrderRequestData.marketOrderResponse = pOpenApi->ordersMarketOrder( orderParams.figi
                                                                                                         , orderParams.getOpenApiOperationType()
                                                                                                         , numLots
                                                                                                         );
                                  }

                                  oss << "Order Request: '" << canonicalOrderRequestString 
                                      <<"': queued with actual value '" << newOrderRequestData.orderAdjustedParams << "'";
                                  terminalData.setStatus(QString::fromStdString(oss.str()));


                                  if (!newInstrument)
                                  {
                                      updateStatusStr();
                                  }
                                  else
                                  {
                                      updateScreen(); // Full update
                                  }
                                  

                                  return false; // Allow new input

                              };


    auto onEditUpdateView = [&]( const tkf::SimpleTerminalLineEditImplBase *pEdit, const std::string &text )
                              {
                                  printEditorText( text, pEdit->getAclt() );
                              };

    
    auto lineEdit = tkf::makeSimpleTerminalLineEdit( onEditTextModified, onEditTextComplete, onEditUpdateView );

    lineEdit.setCaseConvert(1); // upper case

    pTerminalInputEdit = &lineEdit;


    auto onConnected = [&]()
            {
            
                fConnected.store( true, std::memory_order_seq_cst  );

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

                terminalData.setConnectionState( trd::TradingTerminalData::ConnectionState::connected );

                updateStatusStr();

            };

    auto onDisconnected = [&]()
            {
                terminalData.setConnectionState( trd::TradingTerminalData::ConnectionState::disconnected );

                fConnected.store( false, std::memory_order_seq_cst  );

                // Try to reconnect

                webSocket.open( pOpenApi->getStreamingApiNetworkRequest() );

                //terminalData.setConnectionState( tkf::TradingTerminalData::ConnectionState::connecting );

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

                    updateFigiScreen(marketGlass.figi);
                }

                else if (eventName=="instrument_info")
                {
                    tkf::StreamingInstrumentInfoResponse response;
                    response.fromJson(msg);

                    tkf::MarketInstrumentState instrState = tkf::MarketInstrumentState::fromStreamingInstrumentInfoResponse( response );

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

                                               if (terminalData.isFigiChanged() && ordersResponse==0)
                                                   ordersResponse = pOpenApi->orders();

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


    auto checkActiveOrderRequests = [&]()
                                    {
                                        // std::map< std::string, tkf::OrderRequestData > aliveRequests

                                        std::map< std::string, tkf::OrderRequestData >::const_iterator it = activeOrderRequests.begin();

                                        for( ; it!=activeOrderRequests.end(); ++it)
                                        {
                                            if (!it->second.isFinished())
                                                continue;

                                            if (!it->second.isResultValid())
                                            {
                                                std::ostringstream oss;

                                                oss << "Order Request: '" << it->first << "' (completed in " << it->second.operationTimer.elapsed() << "ms): "
                                                    << "Error: ";

                                                if (it->second.isLimitOrderRequest())
                                                {
                                                    oss << it->second.limitOrderResponse->getErrorMessage().toStdString();
                                                }
                                                else
                                                {
                                                    oss << it->second.marketOrderResponse->getErrorMessage().toStdString();
                                                }
                                            
                                                terminalData.setStatus(QString::fromStdString(oss.str()));
                                                updateStatusStr();

                                                activeOrderRequests.erase( it );

                                                return; // Остальное - как-нибудь потом

                                            }

                                            // Good state

                                            tkf::PlacedOrderInfo   placedOrderInfo;

                                            if (it->second.isLimitOrderRequest())
                                            {
                                                placedOrderInfo = tkf::PlacedOrderInfo::fromOrderResponse(it->second.limitOrderResponse->result());
                                            }
                                            else
                                            {
                                                placedOrderInfo = tkf::PlacedOrderInfo::fromOrderResponse(it->second.marketOrderResponse->result());
                                            }

                                            std::ostringstream oss;

                                            oss << "Order Request: '" << it->first << "' (completed in " << it->second.operationTimer.elapsed() << "ms): "
                                                << "Success. "
                                                << "Trk ID: " << placedOrderInfo.trackingId.toStdString() << ". "
                                                << "Op Status: " << placedOrderInfo.status.toStdString() << ". "
                                                << "Order ID: " << placedOrderInfo.orderId.toStdString() << ". "
                                                << "Order Status: " << placedOrderInfo.orderStatus.asJson().toStdString() << ". "
                                                ;

                                            terminalData.setStatus(QString::fromStdString(oss.str()));
                                            updateStatusStr();

                                            activeOrderRequests.erase( it );

                                            // placedOrderInfo - наверное нужно куда-то сохранить

                                            return; // Остальное - как-нибудь потом
                                        
                                        }
                                    };

                                    

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

    // Запрашиваем операции для всех инструментов

    terminalData.setStatus("Initializing");

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


    terminalData.setConnectionState( trd::TradingTerminalData::ConnectionState::connecting );
    tkf::connectStreamingWebSocket( pOpenApi, webSocket, onConnected, onDisconnected, onMessage );


    pTerminalInputEdit->updateView();


    auto processInput = [&]()
                        {
                            std::vector<int> input = simpleInput.readInput();
                           
                            if (!input.empty()) // to stop only when input is not empty
                               lineEdit.processInput( input );
                        };


    terminalData.setStatus("Initialized");


    while( !ctrlC.isBreaked() )
    {
        QTest::qWait(1);

        bool bUpdateScreen = false;

        checkActiveOrderRequests();

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
                lastOrdersResponseLocalDateTime = QDateTime::currentDateTime();
                terminalData.update( activeOrders );

                /// !!! Нужно сравнить ордера по фигам, и найти те фиги, где поменялось
                bUpdateScreen = true;
                
            }
        } // ordersResponse


        if (portfolioResponse && portfolioResponse->isFinished())
        {
            std::map< QString, OpenAPI::PortfolioPosition >  portfolioPositions;

            if (!tkf::processCompletedPortfolioResponse(portfolioResponse, portfolioPositions))
            {
                portfolioResponse = pOpenApi->portfolio(); // rerequest
            }
            else
            {
                portfolioResponse = 0;
                lastPortfolioResponseLocalDateTime = QDateTime::currentDateTime();
                terminalData.update( portfolioPositions );
                bUpdateScreen = true;
            }
        } // portfolioResponse


        if (portfolioCurrenciesResponse && portfolioCurrenciesResponse->isFinished())
        {
            std::map< QString, OpenAPI::CurrencyPosition >  currencyPositions;

            if (!tkf::processCompletedPortfolioCurrenciesResponse(portfolioCurrenciesResponse, currencyPositions))
            {
                portfolioCurrenciesResponse = pOpenApi->portfolioCurrencies(); // rerequest
            }
            else
            {
                portfolioCurrenciesResponse = 0;
                lastPortfolioCurrenciesResponseLocalDateTime = QDateTime::currentDateTime();
                terminalData.update( currencyPositions );
                bUpdateScreen = true;
            }
        } // portfolioCurrenciesResponse


        if (bUpdateScreen)
        {
            updateScreen();
        }


        processInput();


        //------------------------------

        QDateTime dtNow = QDateTime::currentDateTime();



        if ( (dtNow.toMSecsSinceEpoch() - lastOrdersResponseLocalDateTime.toMSecsSinceEpoch()) > 10000)
        {
            // Список заявок не обновлялся больше 30 секунд
            if (ordersResponse==0)
            {
                ordersResponse = pOpenApi->orders(); // rerequest
            }
        }


        if ( (dtNow.toMSecsSinceEpoch() - lastPortfolioResponseLocalDateTime.toMSecsSinceEpoch()) > 10000)
        {
            // Портфель не обновлялся больше 30 секунд
            if (portfolioResponse==0)
            {
                portfolioResponse = pOpenApi->portfolio(); // rerequest
            }
        }

        if ( (dtNow.toMSecsSinceEpoch() - lastPortfolioCurrenciesResponseLocalDateTime.toMSecsSinceEpoch()) > 10000)
        {
            // Портфель не обновлялся больше 30 секунд
            if (portfolioCurrenciesResponse==0)
            {
                portfolioCurrenciesResponse = pOpenApi->portfolioCurrencies(); // rerequest
            }
        }


    } // while( !ctrlC.isBreaked() )



    //TODO: !!!

    /*

       1) Нужно бы сделать настройку логов, и объект, который кутишные логи логгирует.
          Никакой ротации? Или как сделаем, при каждом сообщении - переоткрываем, а ротацию, кто хочет, делает сторонними средствами?

          - логи сделаны, но пока без ротации


       2) Нужно оптимизировать отображение, это не дело, что сейчас происходит.

          - чутка сделано



       3) Нужен ввод заявок - начал думать

          - сделано



       4) Нужно подумать по поводу цветов - монохром очень скучен

          - сделано

          Вариант - считываем QStringList, и затем ручками парсим, формат примерно такой:
            
            red, bg-white, bright, invert, blink

            где: red       - цвет символа
                 bg-white  - цвет фона (bgwhite, bg_white также валидны)
                 bright    - яркость
                 invert    - инверсия фона и цвета символа
                 blink     - мигающий

            цвет фона, яркость, инверсия, мигание - опциональны

            bright, invert, blink - флаги

            bright - вроде работает в виндовой консоли, invert, blink - хз - затачивалось 
            всё под возможности ANSI-терминалов с Escape-последовательностями, 
            для работы с STMки по UARTу с терминалом типа putty



       5) Ещё нужен процентик, насколько текущая цена больше/меньше средней по портфелю.
          Оно ещё влезает, а ещё что-то уже совсем с трудом
          Тут надо обдумать, какие индикаторы нужны в первую очередь.

          а) Процент разницы между текущей ценой и ценой в портфеле - это общий доход/убыток по содержимому портфеля.

          б) Нужно уметь определять, какая из операций последняя - покупка или продажа.
             Затем вычисляем процент изменения цены по сравнению с последней операцией - сравниваем с текущей.
             В таблице этот процент отображаем между покупкой и продажей. Тут надо сделать, чтобы было понятно,
             относительно чего посчитано текущее значение процента.

             Для этого заводим отдельный тип колонки - CUR_PRICE_TO_LAST_BUY_MARKER / CUR_PRICE_TO_LAST_SELL_MARKER

             Порядок столбцов в таблице: LAST_BUY - PERCENT - LAST_SELL - аналогично стакану 
             - там заявки на покупку снизу слева, заявки на продажу - сверху справа.

             CUR_PRICE_TO_LAST_BUY_MARKER  - выводит "<-" - стрелку к покупке, если покупка была последней операцией
             CUR_PRICE_TO_LAST_SELL_MARKER - выводит "->" - стрелку к продаже, если продажа была последней операцией

             И тот и другой маркер ничего не выводят, если последняя операция была не его

             Такие маркеры зависят от расположения колонок, к которым они относятся.

             Для каждого столбца заведём параметр *.swap-direction = true | false
             варианты:

                 *.direction - forward | reverse.
                 *.direction - normal | swap.


             Последний (normal | swap) вроде логичнее всего, но *.swap-direction - проще в реализации - используем обычный bool.

             Суть этого поля в следующем: все нормальные поля его благополучно игнорируют, его используют только те поля, которые 
             выводят указующие маркеры.

             Таким образом, если у нас порядок полей LAST_BUY - PERCENT - LAST_SELL - то ничего в конфиг не добавляем.
             Если же захотелось изменить порядок на обратный: LAST_SELL - PERCENT - LAST_BUY, то в конфиг надо будет добавить:

                 terminal.columns.LAST_SELL.swap-direction = true
                 terminal.columns.LAST_BUY.swap-direction  = true



       6) Поля, в которых сейчас отображается количество лотов, а надо бы - количество акций - 
          я хочу полностью от этих сраных лотов отвязаться - с ним постоянно путанница.

          Q_Buy, Q_Sell - вроде уже в штуках
          Q_Bid, Q_Ask  - в лотах - остались только эти, похоже

          - сделано



       7) Нужно ещё обдумать защиты от дурака - когда забываешь поставить пробел между количеством и ценой,
          и получаешь огромное количество, которое продается по текущей цене.



       8) Символы
          ( *   - дома
          } ;   - на работе (ну или наоборот)

          фантомно возникают.

          UPD - возникали на рабочем компе одни символы, на домашнем - другие.
                По первости - в первый день, может второй день после того, какаяя запилил ввод. 
                Потом все эти фантомы пропали и всё работает нормально.



       9) Выделить Connection Status в отдельное поле, самое первое, наверное. Иначе остальные статусы затирают его,
          а интересно, когда соединение потерялось, когда возобновилось, и что с ним сейчас.



       10) Почему-то LastBuyPr/LastSellPr не обновляются.
           После окончания торгов почему-то не убрались заявки - Q_Bid/CurBidPr и CurAskPr/Q_Ask

           Перво-наперво надо с этим разобраться.


       11) Чего-то висит Best_Bid по LSNGP, хотя вроде должен был очистится

     */
    

    return 0;

    
}



