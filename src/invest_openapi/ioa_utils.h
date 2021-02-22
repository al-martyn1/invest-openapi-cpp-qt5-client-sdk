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
#include <vector>
#include <algorithm>

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

    //QChar chMaxPeriodScale = intervalMaxPeriodStr.back();
    char chMaxPeriodScale = intervalMaxPeriodStr.back().toLatin1();

    intervalMaxPeriodStr.chop(1);

    if (intervalMaxPeriodStr.isEmpty())
        throw std::runtime_error("Invalid Candle Request Interval Limit (2)");

    bool convertRes = false;

    int intervalMaxPeriod = (int)intervalMaxPeriodStr.toUInt( &convertRes, 10 );

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
template< typename OpenApiStructType >
inline
std::vector< OpenApiStructType > makeVectorFromList( const QList<OpenApiStructType> &lst )
{
    std::vector< OpenApiStructType > res = std::vector< OpenApiStructType >( lst.begin(), lst.end() );
    return res;
}

//----------------------------------------------------------------------------
template< typename OpenApiStructType, typename Compare >
inline
std::vector< OpenApiStructType > makeOrderedVectorFromList( const QList<OpenApiStructType> &lst, Compare cmp )
{
    std::vector< OpenApiStructType > res = std::vector< OpenApiStructType >( lst.begin(), lst.end() );
    std::stable_sort( res.begin(), res.end(), cmp );
    return res;
}

//----------------------------------------------------------------------------
template< typename OpenApiStructType >
struct WithGetDateLess
{
    bool operator()( const OpenApiStructType &v1, const OpenApiStructType &v2 ) const
    {
        return v1.getDate() < v2.getDate();
    }
};

//------------------------------
template< typename OpenApiStructType >
struct WithGetDateGreater
{
    bool operator()( const OpenApiStructType &v1, const OpenApiStructType &v2 ) const
    {
        return v1.getDate() > v2.getDate();
    }
};

template< typename OpenApiStructType >
struct WithGetTimeLess
{
    bool operator()( const OpenApiStructType &v1, const OpenApiStructType &v2 ) const
    {
        return v1.getTime() < v2.getTime();
    }
};

//------------------------------
template< typename OpenApiStructType >
struct WithGetTimeGreater
{
    bool operator()( const OpenApiStructType &v1, const OpenApiStructType &v2 ) const
    {
        return v1.getTime() > v2.getTime();
    }
};

//----------------------------------------------------------------------------






//----------------------------------------------------------------------------

} // namespace invest_openapi

