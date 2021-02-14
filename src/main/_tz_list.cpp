/*! \file
    \brief Lists all available timezones and it's aliases

 */

#include <iostream>
#include <exception>
#include <stdexcept>

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

#include "invest_openapi/invest_openapi.h"
#include "invest_openapi/factory.h"
#include "invest_openapi/openapi_completable_future.h"
#include "invest_openapi/qt_time_helpers.h"




inline
std::ostream& printTimeZoneInfo( const QByteArray &tzId )
{
    QTimeZone timeZone = QTimeZone(tzId);
    QDateTime dtNow    = QDateTime::currentDateTime();

    std::cout // << "TZ ID: " << tzId.toStdString()
              // << " - "
              // << timeZone.abbreviation(dtNow).toStdString()
              // << " - "
              // << timeZone.displayName( QTimeZone::GenericTime ).toStdString()
              // << " - "
              << timeZone.comment().toStdString()
              // << " - "
              // << " Has DST: " << ( timeZone.hasDaylightTime() ? "Yes" : "No" )
              //<< endl
              ;

    return std::cout;

}


INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("_tz_list");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;

    cout << "Timezones well-known alias list:" << endl;
    // Timezones well-known alias list
    // Well-known timezones alias list

    std::vector<std::string> tzWlknList = qt_helpers::getTimezonesAliasList<std::string>();

    for( const auto &tzWlkn : tzWlknList )
    {
        cout << "  " << tzWlkn;
        cout << " - " << qt_helpers::getTimezoneAliasDesciption( tzWlkn ).toStdString();
        cout << endl;
    }

    cout << endl;
    cout << "Timezones:" << endl;

    QList<QByteArray> allTzIdList = QTimeZone::availableTimeZoneIds();
    for( auto tzId : allTzIdList )
    {
        cout << "  ";
        cout << tzId.toStdString();
        cout << " - ";
        printTimeZoneInfo( tzId );
        cout<<endl;
    }

    // timeZone.comment().toStdString()

    
    return 0;
}



