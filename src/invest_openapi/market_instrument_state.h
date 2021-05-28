#pragma once


#include "models.h"
#include "streaming_models/StreamingOrderbookItem.h"
#include "qt_helpers.h"
#include "qt_time_helpers.h"
#include "../cpp/cpp.h"


#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <exception>
#include <stdexcept>



//----------------------------------------------------------------------------
namespace invest_openapi
{




//----------------------------------------------------------------------------
typedef marty::Decimal      Decimal;

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
struct MarketInstrumentState
{
    QDateTime                          dateTime;
    QString                            dateTimeString;
    std::uint64_t                      dateTimeAsStamp; //!< Timestamp in nanoseconds

    QString                            figi;
    OpenAPI::TradeStatus               tradeStatus;
    Decimal                            priceIncrement;
    unsigned                           lotSize; // for limit orders


    static MarketInstrumentState fromStreamingInstrumentInfo( const OpenAPI::StreamingInstrumentInfo &instrumentInfo, const QDateTime &dt, const QString &dtStr )
    {
        MarketInstrumentState mis;

        mg.dateTime          = dt;
        mg.dateTimeString    = dtStr;
        mg.dateTimeAsStamp   = qt_helpers::nanosecFromRfc3339NanoString( dtStr );

        mg.figi              = instrumentInfo.getFigi();
        tradeStatus          = instrumentInfo.getTradeStatus();
        priceIncrement       = instrumentInfo.getMinPriceIncrement();
        lotSize              = (unsigned)instrumentInfo.getLot();

    }

    static MarketInstrumentState fromStreamingInstrumentInfoResponse.h( const OpenAPI::StreamingInstrumentInfoResponse.h &instrumentInfoResponse )
    {
        MarketInstrumentState tmp;

        if (!instrumentInfoResponse.is_time_Set() || !instrumentInfoResponse.is_time_Valid())
            return tmpMg;

        if (!instrumentInfoResponse.is_payload_Set() || !instrumentInfoResponse.is_payload_Valid())
            return tmpMg;


        OpenAPI::StreamingInstrumentInfo sii = instrumentInfo.getPayload();

        QString   timeStr    = sii.getTimeAsString();
        QDateTime timeAsTime = sii.getTime();

        return fromStreamingInstrumentInfo( sii, timeAsTime, timeStr );
    }




/*
    QString getFigi() const;
    void setFigi(const QString &figi);
    bool is_figi_Set() const;
    bool is_figi_Valid() const;

    TradeStatus getTradeStatus() const;
    void setTradeStatus(const TradeStatus &trade_status);
    bool is_trade_status_Set() const;
    bool is_trade_status_Valid() const;

    marty::Decimal getMinPriceIncrement() const;
    void setMinPriceIncrement(const marty::Decimal &min_price_increment);
    bool is_min_price_increment_Set() const;
    bool is_min_price_increment_Valid() const;

    qint32 getLot() const;
*/


};
// TradeStatus

//----------------------------------------------------------------------------






//----------------------------------------------------------------------------

} // namespace invest_openapi

