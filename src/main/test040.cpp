/*! \file
    \brief 
 */

//NOTE: Umba headers must be first, at least "umba/umba.h"
#include "umba/umba.h"
#include "umba/char_writers.h"
#include "umba/simple_formatter.h"

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

#include "invest_openapi/terminal_input.h"

#include "invest_openapi/order_params.h"


umba::StdStreamCharWriter coutWriter(std::cout);
umba::StdStreamCharWriter cerrWriter(std::cerr);
umba::NulCharWriter       nulWriter;

umba::SimpleFormatter          tout(&coutWriter); // terminal out - like cout



INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("test040");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    // Custom code goes here

    namespace tkf=invest_openapi;
    
    using namespace umba::omanip;

    // UMBA_TERM_COLORS_MAKE_COMPOSITE( fgColor, bgColor, fBright, fInvert, fBlink )

    auto normalColor        = UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::white   , umba::term::colors::black, false, false, false );
    auto statusColor        = UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::white   , umba::term::colors::black, false, false, false );
    auto captionColor       = UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::yellow  , umba::term::colors::black, false, false, false );
    auto editorColor        = UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::cyan    , umba::term::colors::black, false, false, false );
    //auto editorCursorColor  = UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::black , umba::term::colors::magenta, false, false, false );
    auto editorCursorColor  = UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::black   , umba::term::colors::cyan , false, false, false );

    auto redColor           = UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::red     , umba::term::colors::black, false, false, false );
    auto greenColor         = UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::green   , umba::term::colors::black, false, false, false );
    auto yellowColor        = UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::yellow  , umba::term::colors::black, false, false, false );
    auto blueColor          = UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::blue    , umba::term::colors::black, false, false, false );
    auto magentaColor       = UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::magenta , umba::term::colors::black, false, false, false );
    auto cyanColor          = UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::cyan    , umba::term::colors::black, false, false, false );
    auto whiteColor         = UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::white   , umba::term::colors::black, false, false, false );

    auto redBrColor         = UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::red     , umba::term::colors::black, true , false, false );
    auto greenBrColor       = UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::green   , umba::term::colors::black, true , false, false );
    auto yellowBrColor      = UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::yellow  , umba::term::colors::black, true , false, false );
    auto blueBrColor        = UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::blue    , umba::term::colors::black, true , false, false );
    auto magentaBrColor     = UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::magenta , umba::term::colors::black, true , false, false );
    auto cyanBrColor        = UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::cyan    , umba::term::colors::black, true , false, false );
    auto whiteBrColor       = UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::white   , umba::term::colors::black, true , false, false );
    //auto Color           = UMBA_TERM_COLORS_MAKE_COMPOSITE( umba::term::colors::   , umba::term::colors::black, false, false, false );




    console_helpers::SimpleHandleCtrlC ctrlC; // ctrlC.isBreaked()
    tkf::SimpleTerminalInput simpleInput;

    auto onEditTextUpdateView = [&]( const tkf::SimpleTerminalLineEditImplBase *pEdit, const std::string &text )
                              {
                                  if (!pEdit) return;
                                  tout << term::move2line0;
                                  tout << ">"; // Prompt
                                  tout << color(greenColor) << text << color(redColor) << " " << color(normalColor) << " " << term::clear(2);
                              };

    auto onEditTextModified = [&]( tkf::SimpleTerminalLineEditImplBase *pEdit, std::string &text )
                              {
                                  if (text=="AUTO")
                                      pEdit->setAclt("COMPLETION TEXT");
                                  else if (text=="R")
                                      pEdit->setAclt("OSN");
                              };

    auto onEditTextComplete = [&](tkf::SimpleTerminalLineEditImplBase *pEdit, std::string &text )
                              {
                                  tout << term::move2line0;

                                  auto tmp = tkf::mergeOrderParamsString(tkf::splitOrderParamsString( tkf::prepareOrderParams(text) ));
                                  tout << tmp << endl;

                                  return false;
                              };

    auto lineEdit = tkf::makeSimpleTerminalLineEdit( onEditTextModified, onEditTextComplete, onEditTextUpdateView );

    lineEdit.setCaseConvert(1); // upper case


    // onEditTextUpdateView( 0, std::string() );
    lineEdit.updateView();

    while( !ctrlC.isBreaked() )
    {
        QTest::qWait(1);

        std::vector<int> input = simpleInput.readInput();

        if (!input.empty()) // to stop only when input is not empty
           lineEdit.processInput( input );

    }

    
    return 0;
}



