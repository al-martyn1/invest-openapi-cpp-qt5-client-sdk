#pragma once

#include <QElapsedTimer>
#include <QTest>




namespace invest_openapi
{


//----------------------------------------------------------------------------
inline
void waitOnRequestsLimit( QElapsedTimer &timer, std::size_t requestCounter )
{

    if (requestCounter>0 && (requestCounter%120)==0) // Лимит - 120 запросов в минуту
    {
        std::uint64_t elapsed     = (std::uint64_t)timer.elapsed();
        std::uint64_t interval    = 60*1000u; // 1 минута

        if (elapsed<interval)
        {
            std::uint64_t timeToSleep = interval - elapsed;
            QTest::qWait(timeToSleep);
        }
    }

}








//----------------------------------------------------------------------------


} // namespace invest_openapi

//----------------------------------------------------------------------------
