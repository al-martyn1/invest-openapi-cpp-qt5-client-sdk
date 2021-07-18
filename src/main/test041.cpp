/*! \file
    \brief 
 */

//NOTE: Umba headers must be first, at least "umba/umba.h"
#include "umba/umba.h"
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


#include "invest_openapi/iterable_queue.h"



INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("test041");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;


    // Custom code goes here


    iterable_queue<int> q;

    q.push(3);
    q.push(12);
    q.push(99);
    q.push(45);
    q.push(33);
    q.push(158);
    q.push(67);
    q.push(741);

    auto printQ = [&]( unsigned n )
                  {
                      cout << "[" << n << "] " << "size: " << q.size() << ", " << to_string(q) << endl;

                  };

    printQ(1);


    q.pop();
    q.pop();
    // -2

    printQ(2);

    q.pop();
    q.pop();
    q.pop();
    // -5

    // Сняли больше половины - должно сработать перемещение (в отладочной версии)

    printQ(2);


    q.pop();
    q.pop();
    // -7

    printQ(2);


    q.push(3);
    q.push(12);
    q.push(99);
    q.push(45);
    q.push(33);
    q.push(158);
    q.push(67);
    q.push(741);
    q.push(3);
    q.push(12);
    q.push(99);
    q.push(45);
    q.push(33);
    q.push(158);
    q.push(67);
    q.push(741);
    // +16

    printQ(2);

    // Вроде работает, на первый взгляд
    
    return 0;

}



