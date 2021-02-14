/*! \file
    \brief Qt Date & time tests

 */

#include <iostream>
#include <exception>
#include <stdexcept>
#include <vector>

#include <QCoreApplication>
#include <QString>
#include <QSettings>
#include <QTest>
#include <QDir>
#include <QElapsedTimer>
#include <QTimeZone>
#include <QDateTime>
#include <QLocale>

#include "invest_openapi/config_helpers.h"
#include "invest_openapi/api_config.h"
#include "invest_openapi/auth_config.h"
#include "invest_openapi/currencies_config.h"

#include "invest_openapi/invest_openapi.h"
#include "invest_openapi/factory.h"
#include "invest_openapi/openapi_completable_future.h"
#include "invest_openapi/qt_time_helpers.h"

//----------------------------------------------------------------------------

/*
    Links to read:
      https://stackoverflow.com/questions/21976264/qt-isodate-formatted-date-time-including-timezone
      https://stackoverflow.com/questions/66186141/is-there-the-way-to-parse-time-with-timezone-in-qt/66188970
      https://stackoverflow.com/questions/55370667/qt-qdatetime-from-string-with-timezone-and-daylight-saving

      https://stackoverflow.com/questions/24909579/how-to-obtain-a-list-of-cities-and-countries-in-qt

*/

// QLocale lc = QLocale();
// QLocale lc = QLocale::c();
// QLocale lc = QLocale(QLocale::C);
QLocale lc = QLocale(QLocale::English);




//----------------------------------------------------------------------------
inline
std::string timeSpecToString( Qt::TimeSpec spec)
{
    switch (spec)
    {
        case Qt::LocalTime     : return "Qt::LocalTime";
        case Qt::UTC           : return "Qt::UTC";
        case Qt::OffsetFromUTC : return "Qt::OffsetFromUTC";
        case Qt::TimeZone      : return "Qt::TimeZone";
        default                : return "Qt::TimeSpec Unknown";
    };
}


inline
void printTimeZone( const std::string title, const QTimeZone &tzi )
{
    if (!title.empty())
        std::cout << title << ": ";

    std::cout << tzi.displayName( QTimeZone::GenericTime, QTimeZone::ShortName , lc ).toStdString() 
              << " - "
              << tzi.displayName( QTimeZone::GenericTime, QTimeZone::LongName  , lc ).toStdString() 
              << " - "
              << tzi.displayName( QTimeZone::GenericTime, QTimeZone::OffsetName, lc ).toStdString()
              //<< std::endl
              ;
}

inline
void printTimeZoneInfo( const std::string title, const QByteArray &tzId )
{
    QTimeZone timeZone = QTimeZone(tzId);
    QDateTime dtNow    = QDateTime::currentDateTime();

    if (!title.empty())
        std::cout << title << ": ";

    std::cout << "TZ ID: " << tzId.toStdString()
              << " - "
              << timeZone.abbreviation(dtNow).toStdString()
              << " - "
              << timeZone.displayName( QTimeZone::GenericTime, QTimeZone::DefaultName, lc ).toStdString()
              << " - "
              << timeZone.comment().toStdString()
              << " - "
              << " Has DST: " << ( timeZone.hasDaylightTime() ? "Yes" : "No" )
              //<< endl
              ;
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("test016");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;


    cout<<"Path to exe   : "<<QCoreApplication::applicationDirPath().toStdString()<<endl;

    cout << endl;

    namespace tkf=invest_openapi;

    QDateTime dtNow    = QDateTime::currentDateTime();
    QDateTime dtNowUtc = QDateTime::currentDateTimeUtc();

    //QDateTime::fromString(const QString &string, Qt::DateFormat format = Qt::TextDate)
    //QTime::fromString(const QString &string, Qt::DateFormat format = Qt::TextDate)


    // cout << "Date/Time now      : " << dtNow   .toString("yyyy-MM-ddThh:mm:ss.zzz").toStdString() << endl;
    // cout << "Date/Time now (UTC): " << dtNowUtc.toString("yyyy-MM-ddThh:mm:ss.zzz").toStdString() << endl;

    // https://en.wikipedia.org/wiki/Pacific_Time_Zone
    // PTZ/PST/PDT
    // https://en.wikipedia.org/wiki/List_of_time_zone_abbreviations

    // Североамериканское восточное время (англ. Eastern Standard Time (EST);
    // https://ru.wikipedia.org/wiki/%D0%A1%D0%B5%D0%B2%D0%B5%D1%80%D0%BE%D0%B0%D0%BC%D0%B5%D1%80%D0%B8%D0%BA%D0%B0%D0%BD%D1%81%D0%BA%D0%BE%D0%B5_%D0%B2%D0%BE%D1%81%D1%82%D0%BE%D1%87%D0%BD%D0%BE%D0%B5_%D0%B2%D1%80%D0%B5%D0%BC%D1%8F
    // https://ru.wikipedia.org/wiki/Nasdaq
    // https://ru.wikipedia.org/wiki/%D0%9D%D1%8C%D1%8E-%D0%99%D0%BE%D1%80%D0%BA%D1%81%D0%BA%D0%B0%D1%8F_%D1%84%D0%BE%D0%BD%D0%B4%D0%BE%D0%B2%D0%B0%D1%8F_%D0%B1%D0%B8%D1%80%D0%B6%D0%B0
    // https://en.wikipedia.org/wiki/Boston_Stock_Exchange

    // https://ffin.ru/market/directory/stocks/
    // https://ffin.ru/market/directory/stocks/16327/




    // ISO 8601
    cout << "ISO format" << endl;
    cout << "Date/Time now      : " << dtNow   .toString(Qt::ISODateWithMs).toStdString() << ", TZ: " << dtNow   .timeZoneAbbreviation().toStdString() << ", TimeSpec: " << timeSpecToString(dtNow   .timeSpec()) << endl;
    cout << "Date/Time now (UTC): " << dtNowUtc.toString(Qt::ISODateWithMs).toStdString() << ", TZ: " << dtNowUtc.timeZoneAbbreviation().toStdString() << ", TimeSpec: " << timeSpecToString(dtNowUtc.timeSpec()) << endl;
    cout << endl;

    cout << "RFC format" << endl;
    cout << "Date/Time now      : " << dtNow   .toString(Qt::RFC2822Date).toStdString() << ", TZ: " << dtNow   .timeZoneAbbreviation().toStdString() << ", TimeSpec: " << timeSpecToString(dtNow   .timeSpec()) << endl;
    cout << "Date/Time now (UTC): " << dtNowUtc.toString(Qt::RFC2822Date).toStdString() << ", TZ: " << dtNowUtc.timeZoneAbbreviation().toStdString() << ", TimeSpec: " << timeSpecToString(dtNowUtc.timeSpec()) << endl;
    cout << endl;

    printTimeZone( "Current Time zone      ", dtNow   .timeZone() );
    cout << endl;
    printTimeZone( "Current Time zone (UTC)", dtNowUtc.timeZone() );
    cout << endl;


    // [static]QTimeZone QTimeZone::utc()
    //cout << "Set TZ" << endl;
    printTimeZone( "Set TZ", QTimeZone::systemTimeZone() );
    dtNow.setTimeZone(QTimeZone::systemTimeZone());
    cout << endl;

    cout << "ISO format" << endl;
    cout << "Date/Time now      : " << dtNow   .toString(Qt::ISODateWithMs).toStdString() << ", TZ: " << dtNow   .timeZoneAbbreviation().toStdString() << ", TimeSpec: " << timeSpecToString(dtNow   .timeSpec()) << endl;
    cout << "Date/Time now (UTC): " << dtNowUtc.toString(Qt::ISODateWithMs).toStdString() << ", TZ: " << dtNowUtc.timeZoneAbbreviation().toStdString() << ", TimeSpec: " << timeSpecToString(dtNowUtc.timeSpec()) << endl;
    cout << endl;

    cout << "RFC format" << endl;
    cout << "Date/Time now      : " << dtNow   .toString(Qt::RFC2822Date).toStdString() << ", TZ: " << dtNow   .timeZoneAbbreviation().toStdString() << ", TimeSpec: " << timeSpecToString(dtNow   .timeSpec()) << endl;
    cout << "Date/Time now (UTC): " << dtNowUtc.toString(Qt::RFC2822Date).toStdString() << ", TZ: " << dtNowUtc.timeZoneAbbreviation().toStdString() << ", TimeSpec: " << timeSpecToString(dtNowUtc.timeSpec()) << endl;
    cout << endl;

    printTimeZone( "Current Time zone      ", dtNow   .timeZone() );
    printTimeZone( "Current Time zone (UTC)", dtNowUtc.timeZone() );
    cout << endl;

    cout << endl;
    QTime testTime1 = QTime::fromString( "09:21:16.12 EST", Qt::ISODate );

    cout<<"testTime1: "<<testTime1.toString().toStdString()<<endl;

    cout << endl;
    cout << endl;

    cout << "Timezone aliasing test" << endl;
    std::vector< const char* > tzAliasesTestList = { "RTZ2", "MSK", "KALT", "MSK+2", "CET", "PST", "CT", "AWST", "BGGG", "WAT", "IRST", "SLST", "JST", "Pacific/Truk", "Europe/Mariehamn" };
    for( auto tzAlias : tzAliasesTestList)
    {
        printTimeZoneInfo( tzAlias, qt_helpers::getTimezoneIanaIdFromAlias(tzAlias) );
        cout<<endl;
    }
    

    cout << endl;
    cout << endl;

    cout << "List of timezones" << endl;

    QList<QByteArray> tzIdList = QTimeZone::availableTimeZoneIds();
    for( auto tzId : tzIdList )
    {
        printTimeZoneInfo( std::string(), tzId );
        cout<<endl;
    }

    return 0;

}



