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




umba::StdStreamCharWriter coutWriter(std::cout);
umba::StdStreamCharWriter cerrWriter(std::cerr);
umba::NulCharWriter       nulWriter;



umba::SimpleFormatter          tout(&coutWriter); // terminal out - like cout




INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("test037");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");


    // Custom code goes here

    using namespace umba::omanip;

    tout << "Hello, world!" << endl;

    tout << term::move2down << term::move2down  << term::move2down << term::move2line0;

    tout << term::move2lpos(5) << "Position in line - 5" << endl;

    tout << term::move2pos(7, 10) << "Position - { 7, 10 }" << endl;

    tout << term::move2down;

    tout << "Hello, second world!" << endl;

    console_helpers::SimpleHandleCtrlC ctrlC; // ctrlC.isBreaked()

    int spinnerCounter = 0;

    /*
         Терминальные манипуляторы, в пространстве имен umba::omanip::term

         move2abs0          - Перейти в абсолютный 0.
         move2down          - Перейти ниже на строку на нулевую позицию.
         move2line0         - Перейти в нулевую позицию в текущей строке
         move2lpos(x)       - Перейти на заданную позицию в текущей строке
         move2pos(x,y)      - Переход в абсолютную позицию

         clear_screen       - Очистить весь экран и перейти в 0,0
         clear(int n)       - Очистить несколько строк, начиная с текущей, 0 - не очищать ничего, 1 - остаток текущей строки, >1 - остаток текущей и еще N-1 строк

         spinner(cnt, pos)  - Выводит спиннер. cnt - счётчик спиннера, pos - выводит заданную позицию, не обязательно с нуля выводить его
         spinner_endl       - Завершает вывод спиннера. Очищает старое содержимое, если предыдущий вывод был длинным. Если вывод перенаправлен, выводит endl

     */

    tout << term::spinner(spinnerCounter, 4) << "Very-very-very-very-very-very-very-very-very-very-very-very-very-very-very-very-very-very-very-very-very-very-very-very-very-very-very-very-very-very-very-very long spinner text" << term::spinner_endl;

    while( !ctrlC.isBreaked() )
    {
        //QTest::qWait(100);
        QTest::qWait(1);

        spinnerCounter++;

        tout << term::spinner(spinnerCounter, 4) << "Some long job do here (spinner shown at X=4)" << term::spinner_endl;

    }

    
    return 0;
}



