/*! \file
    \brief 
 */

#pragma once

#include <QObject>
#include <QDebug>
#include <QtDebug>
#include <QTest>
#include <QTimeZone>
#include <QDate>
#include <QTime>
#include <QDateTime>

#include <exception>
#include <stdexcept>

#include "invest_openapi.h"


//----------------------------------------------------------------------------
namespace invest_openapi
{


//----------------------------------------------------------------------------
inline
QDate calcCandleIntervalEnd( QDate intervalStart, QString intervalMaxPeriodStr, const QDate &limitDate )
{
    if (intervalMaxPeriodStr.isEmpty())
        throw std::runtime_error("Invalid Candle Request Interval Limit (1)");

    char chMaxPeriodScale = intervalMaxPeriodStr.back();

    intervalMaxPeriodStr.chop(1);

    if (intervalMaxPeriodStr.isEmpty())
        throw std::runtime_error("Invalid Candle Request Interval Limit (2)");

    bool convertRes = false;

    int intervalMaxPeriod = (int)intervalMaxPeriodStr.toUInt( &convertResk, 10 );

    if (!convertRes || !intervalMaxPeriod || intervalMaxPeriod>10)
        throw std::runtime_error("Invalid Candle Request Interval Limit (3)");

    switch(chMaxPeriodScale)
    {
        case 'd': case 'D':
            intervalStart = intervalStart.addDays(intervalMaxPeriod);
            break;

        case 'y': case 'Y':
            intervalStart = intervalStart.addYears(intervalMaxPeriod);
            break;

        default:
            throw std::runtime_error("Invalid Candle Request Interval Limit (4)");
    };

    if (intervalStart>limitDate)
         return limitDate;

    return intervalStart;
}

// int	compare(const QString &other, Qt::CaseSensitivity cs = Qt::CaseSensitive) const




//----------------------------------------------------------------------------


} // namespace invest_openapi

