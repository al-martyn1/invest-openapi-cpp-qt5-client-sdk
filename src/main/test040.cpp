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


    console_helpers::SimpleHandleCtrlC ctrlC; // ctrlC.isBreaked()
    tkf::SimpleTerminalInput simpleInput;

    auto onEditTextModified = [&]( tkf::SimpleTerminalLineEditImplBase *pEdit, std::string &text )
                              {
                                  if (text=="AUTO")
                                      pEdit->setAclt("COMPLETION TEXT");
                                  else if (text=="R")
                                      pEdit->setAclt("OSN");

                                  tout << term::move2line0;
                                  tout << text << term::clear(2);
                              };

    auto onEditTextComplete = [&](tkf::SimpleTerminalLineEditImplBase *pEdit, std::string &text )
                              {
                                  tout << term::move2line0;

                                  auto tmp = tkf::mergeOrderParams(tkf::splitOrderParamsString( tkf::prepareOrderParams(text) ));
                                  tout << tmp << endl;

                                  return false;
                              };

    auto lineEdit = tkf::makeSimpleTerminalLineEdit( onEditTextModified, onEditTextComplete );

    lineEdit.setCaseConvert(1); // upper case


    while( !ctrlC.isBreaked() )
    {
        QTest::qWait(1);

        std::vector<int> input = simpleInput.readInput();

        if (!input.empty()) // to stop only when input is not empty
           lineEdit.processInput( input );

    }

    
    return 0;
}



