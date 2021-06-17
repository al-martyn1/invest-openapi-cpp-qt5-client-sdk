#pragma once

#include <QElapsedTimer>
#include <QTest>




namespace invest_openapi
{


struct RequestLimits
{
    const static std::size_t  minuteMillisecs      = 60000u;

    const static std::size_t  maxRequestsPerMinute = 120; // Лимит - 120 запросов в минуту


    static bool isLimitReached( std::size_t requestCounter, std::size_t limitValue )
    {
        if (!requestCounter)
            return false;

        return (requestCounter%limitValue)==0;
    }


}; // struct RequestLimits

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
std::uint64_t calcWaitIntervalOnRequestsLimit( QElapsedTimer &timer, std::size_t requestCounter, std::size_t limitValue = RequestLimits::maxRequestsPerMinute )
{
    if ( RequestLimits::isLimitReached( requestCounter, limitValue ) )
    {
        std::uint64_t elapsed     = (std::uint64_t)timer.elapsed();
        std::uint64_t interval    = RequestLimits::minuteMillisecs;

        if ( elapsed < interval )
        {
            std::uint64_t timeToSleep = interval - elapsed;
            return timeToSleep;
        }
    }

    return 0;

}

//----------------------------------------------------------------------------
inline
void checkWaitOnRequestsLimit( QElapsedTimer &timer, std::size_t requestCounter, std::size_t limitValue = RequestLimits::maxRequestsPerMinute)
{
    auto timeToSleep = calcWaitIntervalOnRequestsLimit( timer, requestCounter, limitValue );
    if (!timeToSleep)
        return;

    QTest::qWait(timeToSleep);
    timer.restart();

}

//----------------------------------------------------------------------------







//----------------------------------------------------------------------------


} // namespace invest_openapi

//----------------------------------------------------------------------------
