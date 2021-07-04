/*! \file
    \brief 
 */

//NOTE: Umba headers must be first, at least "umba/umba.h"
#include "umba/umba.h"
#include "umba/simple_formatter.h"
#include "umba/char_writers.h"

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


INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("test039");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;

    // Custom code goes here
    
    namespace tkf=invest_openapi;

    console_helpers::SimpleHandleCtrlC ctrlC; // ctrlC.isBreaked()
    tkf::SimpleTerminalInput simpleInput;

    while( !ctrlC.isBreaked() )
    {
        QTest::qWait(1);

        std::vector<int> input = simpleInput.readInput();

        for(auto i : input)
        {
            cout << "Input: " << i;

            if (i>=' ' && i<127)
               cout << " '" << (char)i << "'";

            cout << endl;
        }

    }
    
    return 0;
}



