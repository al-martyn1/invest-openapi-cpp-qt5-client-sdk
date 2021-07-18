#pragma once

#include "qt_monotonic_timestamp_ms.h"




namespace invest_openapi
{

inline
std::uint64_t getTimestampMs()
{
    return qtGetMonotonicTimestamp();
}


} // namespace invest_openapi


