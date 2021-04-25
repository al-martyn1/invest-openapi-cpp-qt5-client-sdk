/*! \file
    \brief Configs lookup test

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
#include <iomanip>

#include <QCoreApplication>
#include <QString>
#include <QSettings>
#include <QTest>
#include <QDir>
#include <QElapsedTimer>

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
    QCoreApplication::setApplicationName("test033");
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



    QDateTime dtNow = QDateTime::currentDateTime();

    std::uint64_t msecs     = dtNow.toMSecsSinceEpoch();
    std::string   msecsStr  = std::to_string(msecs);
    cout << "msecs: " << msecsStr << ", size: " << msecsStr.size() << endl;

    //  1 619 373 122 400 000 000 - 0x 1679 2B46 6777 9800
    // 18 446 744 073 709 551 615 - 0x FFFF FFFF FFFF FFFF
    //              4 294 967 295 - 0x FFFF FFFF

    // Запас есть, можно не парится особо

    std::uint64_t nsecs = msecs*1000*1000;           
    std::string   nsecsStr  = std::to_string(nsecs);
    cout << "nsecs: " << nsecsStr << ", size: " << nsecsStr.size() << endl;


    std::uint64_t nsPrev = 0;
    //nsecs = qt_helpers::nanosecFromRfc3339NanoString( QString str )

    std::vector< QString > times = { "2021-04-23T20:49:58.271483332Z" 
                                   , "2021-04-23T20:49:58.27475194Z"  
                                   , "2021-04-23T20:49:58.294388887Z" 
                                   , "2021-04-23T20:49:58.301856135Z" 
                                   , "2021-04-23T20:49:58.303532501Z" 
                                   , "2021-04-23T20:49:58.317959664Z" 
                                   , "2021-04-23T20:49:58.580628049Z"
                                   , "2021-04-23T20:49:58.584247997Z"
                                   , "2021-04-23T20:49:58.921135665Z"
                                   , "2021-04-23T20:49:58.923220281Z"
                                   };

    for( auto strTime : times )
    {
        std::uint64_t ns = qt_helpers::nanosecFromRfc3339NanoString( strTime );
        if (!ns)
        {
            cout << "Time string not parsed, str: " << strTime << endl;
        }
        else
        {
            cout << "Nsec value: " << ns;
            if (nsPrev)
            {
                if (nsPrev>ns)
                {
                    cout << ", delta: invalid";
                }
                else
                {
                    std::uint64_t delta = ns - nsPrev;
                    cout << ", delta: " << std::setw(9) << delta;
                }
            }

            cout << endl;

            nsPrev = ns;

        }
    }

    
    return 0;
}



