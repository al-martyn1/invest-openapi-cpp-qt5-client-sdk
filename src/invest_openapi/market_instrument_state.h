#pragma once


#include "models.h"
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


    bool isValid() const
    {
        return !dateTimeString.isEmpty() && !figi.isEmpty();
    }


    static MarketInstrumentState fromStreamingInstrumentInfo( const OpenAPI::StreamingInstrumentInfo &instrumentInfo, const QDateTime &dt, const QString &dtStr )
    {
        MarketInstrumentState mis;

        mis.dateTime          = dt;
        mis.dateTimeString    = dtStr;
        mis.dateTimeAsStamp   = qt_helpers::nanosecFromRfc3339NanoString( dtStr );

        mis.figi              = instrumentInfo.getFigi().toUpper();
        mis.tradeStatus       = instrumentInfo.getTradeStatus();
        mis.priceIncrement    = instrumentInfo.getMinPriceIncrement();
        mis.lotSize           = (unsigned)instrumentInfo.getLot();

        return mis;
    }

    static MarketInstrumentState fromStreamingInstrumentInfoResponse( const OpenAPI::StreamingInstrumentInfoResponse &instrumentInfoResponse )
    {
        MarketInstrumentState tmp;

        if (!instrumentInfoResponse.is_time_Set() || !instrumentInfoResponse.is_time_Valid())
            return tmp;

        if (!instrumentInfoResponse.is_payload_Set() || !instrumentInfoResponse.is_payload_Valid())
            return tmp;


        OpenAPI::StreamingInstrumentInfo sii = instrumentInfoResponse.getPayload();

        QString   timeStr    = instrumentInfoResponse.getTimeAsString();
        QDateTime timeAsTime = instrumentInfoResponse.getTime();

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
inline
std::ostream& operator<<( std::ostream &s, MarketInstrumentState mis ) 
{
    using std::endl;
    using cpp::makeExpandString;
    using cpp::expandAtFront;
    using cpp::expandAtBack;

    s << "Instrument Info for : " << mis.figi  << endl;
    s << "              Time  : " << mis.dateTimeString  << endl;
    s << "              Stamp : " << mis.dateTimeAsStamp << endl;

    // mis.tradeStatus.getValue() == OpenAPI::TradeStatus::INVALID_VALUE_OPENAPI_GENERATED = 0,
    //                               OpenAPI::TradeStatus::NORMALTRADING
    //                               OpenAPI::TradeStatus::NOTAVAILABLEFORTRADING

    s << "              Status: " << mis.tradeStatus.asJson() << endl;
    s << "              Inc   : " << mis.priceIncrement << endl;
    s << "              Lot   : " << mis.lotSize << endl;
    s << endl;

    return s;
}


//----------------------------------------------------------------------------

} // namespace invest_openapi

