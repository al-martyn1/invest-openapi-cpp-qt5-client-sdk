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

    bool isTradeStatusInvalid       () const  { return tradeStatus.getValue() == OpenAPI::TradeStatus::eTradeStatus::INVALID_VALUE_OPENAPI_GENERATED;  }
    bool isTradeStatusNormalTrading () const  { return tradeStatus.getValue() == OpenAPI::TradeStatus::eTradeStatus::NORMALTRADING;  }
    bool isTradeStatusNotTraded     () const  { return tradeStatus.getValue() == OpenAPI::TradeStatus::eTradeStatus::NOTAVAILABLEFORTRADING;  }


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


}; // struct MarketInstrumentState

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
bool isMarketInstrumentActive( const std::map< QString, MarketInstrumentState > &instrumentStates, const QString &figi )
{
    std::map< QString, MarketInstrumentState >::const_iterator it = instrumentStates.find(figi.toUpper());
    if (it == instrumentStates.end())
        return false;
    
    return it->second.isTradeStatusNormalTrading();
}

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
enum class MarketInstrumentStateChange
{
    becomeNormalTrading,
    becomeNotTraded

};

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
//! Возвращает true, если статус изменился хотя бы у одного инструмента
inline
bool isMarketInstrumentsStateChanged( const std::map< QString, MarketInstrumentState >          &instrumentStatesOld
                                    , const std::map< QString, MarketInstrumentState >          &instrumentStatesNew
                                    , const std::map< QString, MarketInstrumentStateChange >    *pInstrumentStateChanges = 0
                                    )
{
    bool gotChanges = false;

    std::map< QString, MarketInstrumentState >::const_iterator nit = instrumentStatesNew.begin();
    for(; nit != instrumentStatesNew.end(); ++nit )
    {
        QString instrumentFigi = nit->first.toUpper();

        std::map< QString, MarketInstrumentState >::const_iterator oit = instrumentStatesOld.find(instrumentFigi);

        if (oit==instrumentStatesOld.end()) // No insrtrument info have before?
        {
            // There is no old state branch

            if (nit->second.isTradeStatusNormalTrading()) // New state is NormalTrading?
            {
                if (pInstrumentStateChanges)
                {
                    std::map< QString, MarketInstrumentStateChange > &instrumentStateChanges = *pInstrumentStateChanges;
                    instrumentStateChanges[instrumentFigi]                                   =  MarketInstrumentStateChange::becomeNormalTrading;
                }
            }
            else // New state is not NormalTrading - error, or may be somehing else?
            {
                if (pInstrumentStateChanges)
                {
                    std::map< QString, MarketInstrumentStateChange > &instrumentStateChanges = *pInstrumentStateChanges;
                    instrumentStateChanges[instrumentFigi]                                   =  MarketInstrumentStateChange::becomeNotTraded;
                }
            }

            gotChanges = true;

            continue;
        }


        // Hm-m-m. Instument has prev state

        if (oit->second.isTradeStatusNormalTrading() == nit->second.isTradeStatusNormalTrading())
        {
            // We have got a prev state
            continue; // No change
        }


        // State changed
        if (oit->second.isTradeStatusNormalTrading()) // old state is NormalTrading
        {
            instrumentStateChanges[instrumentFigi] = MarketInstrumentStateChange::becomeNotTraded; // new state not normal
        }
        else
        {
            instrumentStateChanges[instrumentFigi] = MarketInstrumentStateChange::becomeNormalTrading; // new state is normal
        }

        gotChanges = true;

    } // for(; nit != instrumentStatesNew.end(); ++nit )

    return gotChanges;

}

//----------------------------------------------------------------------------
//! Возвращает true, если статус изменился хотя бы у одного инструмента
inline
bool isMarketInstrumentsStateChanged( const std::map< QString, MarketInstrumentState >          &instrumentStatesOld
                                    , const std::map< QString, MarketInstrumentState >          &instrumentStatesNew
                                    ,       std::map< QString, MarketInstrumentStateChange >    &instrumentStateChanges
                                    )
{
    return isMarketInstrumentsStateChanged( instrumentStatesOld, instrumentStatesNew, &instrumentStateChanges );
}

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

