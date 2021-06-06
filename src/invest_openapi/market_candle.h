#pragma once


#include "models.h"
#include "streaming_models/StreamingOrderbookItem.h"
#include "qt_helpers.h"
#include "qt_time_helpers.h"
#include "../cpp/cpp.h"

#include "market_math.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <map>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <utility>
#include <iterator>



//----------------------------------------------------------------------------
namespace invest_openapi
{




//----------------------------------------------------------------------------
struct MarketCandle
{

    typedef marty::Decimal      Decimal;


    QDateTime                    eventDateTime;
    QString                      eventDateTimeString;
    std::uint64_t                eventDateTimeAsStamp; //!< Timestamp in nanoseconds


    QString                      figi;
    OpenAPI::CandleResolution    interval;
    QDateTime                    dateTime;

    Decimal                      openPrice;
    Decimal                      closePrice;
    Decimal                      highPrice;
    Decimal                      lowPrice;

    std::int64_t                 volume;


    //------------------------------
    bool isValid() const
    {
        return !figi.isEmpty();
    }



    static 
    MarketCandle fromCandle( const OpenAPI::Candle &candle )
    {
        MarketCandle res;

        if (!candle.isSet() || !candle.isValid())
            return res;

        res.figi       = candle.getFigi();
        res.interval   = candle.getInterval();
        res.dateTime   = candle.getTime();

        res.openPrice  = candle.getO();
        res.closePrice = candle.getC();
        res.highPrice  = candle.getH();
        res.lowPrice   = candle.getL();

        res.volume     = (std::int64_t)candle.getV();

        return res;

    }

    static 
    MarketCandle fromStreamingCandleResponse( const OpenAPI::StreamingCandleResponse &response )
    {
        if (!response.isSet() || !response.isValid())
            return MarketCandle{};

        if (!response.is_payload_Set() || !response.is_payload_Valid())
            return MarketCandle{};

        MarketCandle res = fromCandle( response.getPayload() );

        if (!response.is_time_Set() || !response.is_time_Valid())
            return res;

        res.eventDateTime        = response.getTime();
        res.eventDateTimeString  = response.getTimeAsString();
        res.eventDateTimeAsStamp = qt_helpers::nanosecFromRfc3339NanoString( response.getTimeAsString() );

        return res;

        }


}; // struct MarketCandle







//----------------------------------------------------------------------------






//----------------------------------------------------------------------------

} // namespace invest_openapi

