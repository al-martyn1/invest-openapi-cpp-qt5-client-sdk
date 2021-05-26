#pragma once


#include "models.h"
#include "streaming_models/StreamingOrderbookItem.h"
#include "qt_helpers.h"
#include "qt_time_helpers.h"


#include <iostream>



//----------------------------------------------------------------------------
namespace invest_openapi
{



//----------------------------------------------------------------------------
struct MarketGlassItem
{
    marty::Decimal   price;
    int              quantity;

    static MarketGlassItem fromStreamingOrderbookItem( const OpenAPI::StreamingOrderbookItem &item )
    {
        MarketGlassItem mgi;

        mgi.price     = item.price;
        mgi.quantity  = int(item.quantity);
        
        return mgi;
    }

}; // struct MarketGlassItem

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
struct MarketGlass
{
    QString                            figi;  //!< Instrument FIGI
    unsigned                           depth; //!< Glass depth

    QDateTime                          dateTime;
    QString                            dateTimeString;
    std::uint64_t                      dateTimeAsStamp; //!< Timestamp in nanoseconds

    std::vector< MarketGlassItem >     asks; //!< Запросы на продажу
    std::vector< MarketGlassItem >     bids; //!< Запросы на покупку (предложения)


    bool isValid() const
    {
        return !dateTimeString.isEmpty() && !figi.isEmpty();
    }


    static MarketGlass fromStreamingOrderbook( const OpenAPI::StreamingOrderbook &orderBook, const QDateTime &dt, const QString &dtStr )
    {
        MarketGlass mg;

        mg.figi              = orderBook.getFigi();
        mg.depth             = orderBook.getDepth();

        mg.dateTime          = dt;
        mg.dateTimeString    = dtStr;
        mg.dateTimeAsStamp   = qt_helpers::nanosecFromRfc3339NanoString( strTime );


        if (orderBook.is_asks_Set() && orderBook.is_asks_Valid())
        {
            QList< QList<marty::Decimal> > obAsks = orderBook.getAsks();

            for( const auto &askAsList : obAsks )
            {
                StreamingOrderbookItem obItem = StreamingOrderbookItem::fromList(askAsList);
                mg.asks.push_back( MarketGlassItem::fromStreamingOrderbookItem(obItem) );
            }
        }

        if (orderBook.is_bids_Set() && orderBook.is_bids_Valid())
        {
            QList< QList<marty::Decimal> > obBids = orderBook.getBids();

            for( const auto &bidAsList : obBids )
            {
                StreamingOrderbookItem obItem = StreamingOrderbookItem::fromList(bidAsList);
                mg.bids.push_back( MarketGlassItem::fromStreamingOrderbookItem(obItem) );
            }
        }


        return mg;
    }


    static MarketGlass fromStreamingOrderbookResponse( const OpenAPI::StreamingOrderbookResponse &orderBookResponse )
    {
        MarketGlass tmpMg;

        if (!orderBookResponse.is_time_Set() || !orderBookResponse.is_time_Valid())
            return tmpMg;

        if (!orderBookResponse.is_payload_Set() || !orderBookResponse.is_payload_Valid())
            return tmpMg;


        OpenAPI::StreamingOrderbook orderBook = orderBookResponse.getPayload();

        QString   timeStr    = orderBookResponse.getTimeAsString();
        QDateTime timeAsTime = orderBookResponse.getTime();

        return fromStreamingOrderbook( orderBook, timeAsTime, timeStr );
    }

}; // struct MarketGlass

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
std::ostream& operator<<( std::ostream &s, constd MarketGlass &mgi ) 
{
    return s;
}

//----------------------------------------------------------------------------



/*
struct StreamingOrderbookItem
{
    marty::Decimal   price;
    marty::Decimal   quantity;
*/


//----------------------------------------------------------------------------






//----------------------------------------------------------------------------

} // namespace invest_openapi

