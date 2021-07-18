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
#include <QRandomGenerator>
#include <QTime>

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
#include "invest_openapi/bandwidth_meter.h"
#include "invest_openapi/default_monotonic_timestamper.h"



INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("test042");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;


    // Custom code goes here

    // DefaultMonotonicTimestamper    timestamper;

    BandwidthMeter< DefaultMonotonicTimestamper::timestamp_type
                  , DefaultMonotonicTimestamper
                  , std::size_t
                  > 
                  meter;

    meter.setInterval(10000); // 10 seconds


    QRandomGenerator qrnd; // ::bounded(int highest)

    const QString timeFmt = "HH:mm:ss.zzz";

    console_helpers::SimpleHandleCtrlC ctrlC;

    QElapsedTimer printTimer;
    printTimer.start();

    while( !ctrlC.isBreaked() )
    {
        QTest::qWait(1);

        if ( qrnd.bounded(2) ) // 0 or 1
        {
            meter.push();
        }

        //if (meter.timestamp() % 1000)
        if (printTimer.elapsed()>1000)
        {
            printTimer.restart();
            QTime qtmNow = QTime::currentTime();
            cout << "[" << qtmNow.toString(timeFmt).toStdString() << "]" << " " << "# Events in last 10 seconds: " << meter.size() << endl;
        }
        // qrnd.bounded(2);
        

    }


    
    return 0;
}



