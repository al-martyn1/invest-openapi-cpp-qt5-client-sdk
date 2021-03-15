#pragma once

#include <QTimeZone>
#include <QDate>
#include <QTime>
#include <QString>
#include <QByteArray>
#include <QDateTime>

#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <string>
#include <exception>
#include <stdexcept>

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
namespace qt_helpers
{



//----------------------------------------------------------------------------
inline
QDateTime makeMidnightDateTime( const QDate &date )
{
    auto zeroTime = QTime(0 /* h */, 0 /* m */ , 0 /* s */, 0 /* ms */ );
    return QDateTime(date, zeroTime, Qt::UTC);
}

//----------------------------------------------------------------------------
inline
QDateTime makeMidnightDateTime( const QDateTime &dt )
{
    return makeMidnightDateTime( dt.date() );
}

//----------------------------------------------------------------------------
inline
QString formatUtcOffset( int utcOffset )
{
    QString utcStr;

    if (utcOffset<0)
    {
        utcStr.append("-");
        utcOffset = -utcOffset;
    }
    else
    {
        utcStr.append("+");
    }

    utcOffset /= 60;
    int hours   = utcOffset/60;
    int minutes = utcOffset%60;

    return utcStr + QString::asprintf("%02d:%02d", hours, minutes );
}

//----------------------------------------------------------------------------
inline
QString formatDateTimeISO8601( const QDateTime &dt, bool utcOffsetAuto = false )
{
    int utcOffset = dt.offsetFromUtc();
    QString utcOffsetStr;

    if (!utcOffsetAuto || utcOffset!=0)
        utcOffsetStr = formatUtcOffset(utcOffset);

    return dt.toString("yyyy-MM-ddThh:mm:ss.zzz")
    + QString("000")
    + utcOffsetStr
    ;
}

//----------------------------------------------------------------------------
inline
QDateTime parseDateTimeISO8601( QString dtStr )
{
    /*
    int idxPosT      = dtStr.indexOf('T');
    if (idxPosT<0)
        idxPosT      = dtStr.indexOf(' ');

    int idxPlusSign  = dtStr.indexOf('+');
    int idxMinusSign = dtStr.indexOf('-', idxPosT);

    if (idxPlusSign>=0 && idxMinusSign>=0)
    {
        // Both found, something wrong
        int maxIdx = idxPlusSign;
        if (idxMinusSign>idxPlusSign)
        {
            maxIdx = idxMinusSign;
            idxMinusSign = -1;
        }
        else
        {
            idxPlusSign = -1;
        }

        dtStr.remove( maxIdx, dtStr.size()-maxIdx);
    }

    if (idxPlusSign>=0 || idxMinusSign>=0)
    {
        int signPos = ( idxPlusSign>=0 ? idxPlusSign : idxMinusSign);
    }
    */
    //QDateTime(const QDate &date, const QTime &time, Qt::TimeSpec spec, int offsetSeconds)

    // Нормально парсит и так, не нужно никаких приседаний

    return QDateTime::fromString( dtStr, Qt::ISODateWithMs );
    //return QDateTime();
}

// QDateTime QDateTime::toTimeZone(const QTimeZone &timeZone) const
// QDateTime QDateTime::toUTC()




//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
} // namespace qt_helpers




