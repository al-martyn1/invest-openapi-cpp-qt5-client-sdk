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
#include <utility>



//----------------------------------------------------------------------------
namespace invest_openapi
{




//----------------------------------------------------------------------------
typedef marty::Decimal      Decimal;

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
struct MarketGlassItem
{
    Decimal          price;
    unsigned         quantity;

    static MarketGlassItem fromStreamingOrderbookItem( const OpenAPI::StreamingOrderbookItem &item )
    {
        MarketGlassItem mgi;

        mgi.price     = item.price;
        mgi.quantity  = unsigned(item.quantity);
        
        return mgi;
    }

}; // struct MarketGlassItem

//------------------------------
struct MarketGlassItemPriceGreater
{
    bool operator()( const MarketGlassItem &mgi1, const MarketGlassItem &mgi2 ) const
    {
        return mgi1.price > mgi2.price;
    }
};

//------------------------------
struct MarketGlassItemPriceLess
{
    bool operator()( const MarketGlassItem &mgi1, const MarketGlassItem &mgi2 ) const
    {
        return mgi1.price < mgi2.price;
    }
};

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
struct MarketGlass
{
    QDateTime                          dateTime;
    QString                            dateTimeString;
    std::uint64_t                      dateTimeAsStamp; //!< Timestamp in nanoseconds

    QString                            figi;  //!< Instrument FIGI
    unsigned                           depth; //!< Glass depth

    std::vector< MarketGlassItem >     asks; //!< Запросы на продажу               - отсортированы по убыванию
    std::vector< MarketGlassItem >     bids; //!< Запросы на покупку (предложения) - отсортированы по убыванию


protected:

    unsigned                           quantityAsks;
    unsigned                           quantityBids;


    template< typename IterType >
    static bool isAllQuantityZero( IterType b, IterType e )
    {
        for(; b!=e; ++b)
        {
            if (b->quantity==0)
                continue;
            return false;
        }

        return true;
    }


    bool isAsksQuantityAllZero() const     { return isAllQuantityZero( asks.begin() , asks.end() );  }
    bool isBidsQuantityAllZero() const     { return isAllQuantityZero( bids.begin() , bids.end() );  }


    template< typename IterType >
    Decimal getGlassMinMaxHelper( IterType b, IterType e, const std::string &thMsg ) const 
    {
        for(; b!=e; ++b)
        {
            if (b->quantity==0)
                continue;
            return b->price;
        }

        throw std::runtime_error(std::string("invest_openapi::MarketGlass::") + thMsg);

        return Decimal(0);
    }

    template< typename IterType >
    unsigned calcQuantity( IterType b, IterType e )
    {
        unsigned q = 0;

        for(; b!=e; ++b)
        {
            q += b->quantity;
        }

        return q;
    }

    template <typename T >
    Decimal calcRatio( T dividend, T divisor, int divPrecision ) const
    {
        Decimal res = Decimal(dividend);

        if (res==Decimal(0))
            return res;

        res.div( Decimal(divisor), divPrecision );

        return res;
    }



public:


    bool isValid() const
    {
        return !dateTimeString.isEmpty() && !figi.isEmpty() && !asks.empty() && !bids.empty() && !isAsksQuantityAllZero() && !isBidsQuantityAllZero();
    }

    std::string whyInvalid() const
    {
        if (dateTimeString.isEmpty()) return "Source invalid";
        if (figi.isEmpty())           return "FIGI not received";
        if (asks.empty())             return "Asks is empty";
        if (bids.empty())             return "Bids is empty";
        if (isAsksQuantityAllZero())  return "Asks are all zero";
        if (isBidsQuantityAllZero())  return "Bids are all zero";
        return "Object is fully valid";
    }


    Decimal getGlassMaxPrice() const    { return getGlassMinMaxHelper( asks.begin() , asks.end() , "getGlassMaxPrice - failed to get max price" ); }
    Decimal getGlassMinPrice() const    { return getGlassMinMaxHelper( bids.rbegin(), bids.rend(), "getGlassMinPrice - failed to get min price" ); }

    Decimal getAsksMaxPrice() const     { return getGlassMinMaxHelper( asks.begin() , asks.end() , "getAsksMaxPrice - failed to get max price" ); }
    Decimal getAsksMinPrice() const     { return getGlassMinMaxHelper( asks.rbegin(), asks.rend(), "getAsksMinPrice - failed to get min price" ); }

    Decimal getBidsMaxPrice() const     { return getGlassMinMaxHelper( bids.begin() , bids.end() , "invest_openapi::MarketGlass::getAsksMaxPrice - failed to get max price" ); }
    Decimal getBidsMinPrice() const     { return getGlassMinMaxHelper( bids.rbegin(), bids.rend(), "invest_openapi::MarketGlass::getAsksMinPrice - failed to get min price" ); }

    Decimal getAskBestPrice() const     { return getAsksMinPrice(); }
    Decimal getBidBestPrice() const     { return getBidsMaxPrice(); }
    Decimal getPriceSpread()  const     { return getAskBestPrice() - getBidBestPrice(); }

    int getPriceSpreadPoints( const Decimal &priceStep ) const;

    unsigned getQuantityAsks() const    { return quantityAsks; }
    unsigned getQuantityBids() const    { return quantityBids; }

    Decimal getAsksBidsRatio() const    { return calcRatio( quantityAsks, quantityBids, 3 ); }
    Decimal getBidsAsksRatio() const    { return calcRatio( quantityBids, quantityAsks, 3 ); }



    void updateQuantity()
    {
        quantityAsks = calcQuantity( asks.begin() , asks.end() );
        quantityBids = calcQuantity( bids.begin() , bids.end() );
    }


    static MarketGlass fromStreamingOrderbook( const OpenAPI::StreamingOrderbook &orderBook, const QDateTime &dt, const QString &dtStr )
    {
        MarketGlass mg;

        mg.dateTime          = dt;
        mg.dateTimeString    = dtStr;
        mg.dateTimeAsStamp   = qt_helpers::nanosecFromRfc3339NanoString( dtStr );

        mg.figi              = orderBook.getFigi().toUpper();
        mg.depth             = orderBook.getDepth();


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

        
        // Не расчитываем, что всё придёт в нужном порядке, сортируем сами
        std::stable_sort( mg.asks.begin(), mg.asks.end(), MarketGlassItemPriceGreater() );
        std::stable_sort( mg.bids.begin(), mg.bids.end(), MarketGlassItemPriceGreater() );

        mg.updateQuantity();

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


    int alignPrecision()
    {
        int maxPricePrecision = 0;

        for( const auto &ask: asks )
        {
            int p = ask.price.precision();
            if (maxPricePrecision<p)
                maxPricePrecision = p;
        }
       
        for( const auto &bid: bids )
        {
            int p = bid.price.precision();
            if (maxPricePrecision<p)
                maxPricePrecision = p;
        }

        for( auto &ask: asks )
            ask.price.precisionExpandTo(maxPricePrecision);
       
        for( auto &bid: bids )
            bid.price.precisionExpandTo(maxPricePrecision);

        return maxPricePrecision;
    }

    MarketGlass getPrecisionAlignedGlass() const
    {
        MarketGlass cp = *this;
        cp.alignPrecision();
        return cp;
    }


}; // struct MarketGlass

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
int MarketGlass::getPriceSpreadPoints( const Decimal &priceStep ) const
{
    Decimal spread = getPriceSpread();

    /*
    #if defined(DEBUG) || defined(_DEBUG)
        Decimal     spreadCnt        = spread / priceStep;
        std::string spreadPointsStr  = spreadCnt.to_string();
    #endif
    */

    bool neg = false;

    if (spread.sgn()<0)
    {
        neg    = true;
        spread = -spread;
    }

    int spreadPoints = 0;

    Decimal spreadCounter = Decimal(0);

    while(spreadCounter<=spread)
    {
        ++spreadPoints;
        spreadCounter += priceStep;
    }

    spreadPoints -= 1;

    return neg ? -spreadPoints : spreadPoints;
}


//----------------------------------------------------------------------------
inline
std::string marketGlassFormatHelperForPrice( const std::string &priceStr, std::size_t maxPriceWidth )
{
    using cpp::makeExpandString;
    using cpp::expandAtFront;
    using cpp::expandAtBack;

    std::size_t priceWidth = priceStr.size();

    if (priceWidth>=maxPriceWidth)
        return priceStr;

    std::size_t rightFieldWidth = (maxPriceWidth - priceWidth) / 2;

    std::string resTmp = expandAtBack( priceStr, priceWidth+rightFieldWidth );

    return expandAtFront( resTmp, maxPriceWidth );
}

//----------------------------------------------------------------------------
inline
std::string marketGlassFormatHelperForPrice( marty::Decimal price, int precision, std::size_t maxPriceWidth )
{
    price.precisionExpandTo(precision);

    std::string strPrice = price.to_string(precision);

    return marketGlassFormatHelperForPrice( strPrice, maxPriceWidth );
}

//----------------------------------------------------------------------------
inline
std::string marketGlassFormatHelperForQuantity( int q, std::size_t w )
{
    using cpp::expandAtFront;
    using cpp::expandAtBack;

    std::ostringstream oss;
    oss << q;

    return expandAtFront( oss.str(), w );
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
std::ostream& operator<<( std::ostream &s, MarketGlass mg ) 
{
    using std::endl;
    using cpp::makeExpandString;
    using cpp::expandAtFront;
    using cpp::expandAtBack;

    s << "Glass for  : " << mg.figi  << endl;
    s << "      Depth: " << mg.depth << endl;
    s << "      Time : " << mg.dateTimeString  << endl;
    s << "      Stamp: " << mg.dateTimeAsStamp << endl;
    s << endl;

    std::size_t priceFieldWidth = 4;


    /*
               price    askQ
      ------------------------
        bidQ   price
    
    */


    //int         maxPricePrecision = 0;
    std::size_t maxPriceWidth     = 0;
    std::size_t sepWidth          = 2;

    //------------------------------
    /*
    for( auto ask: mg.asks )
    {
        int p = ask.price.precision();
        if (maxPricePrecision<p)
            maxPricePrecision = p;
    }

    for( auto bid: mg.bids )
    {
        int p = bid.price.precision();
        if (maxPricePrecision<p)
            maxPricePrecision = p;
    }
    */
    //------------------------------

    //MarketGlass getPrecisionAlignedGlass() const

    int maxPricePrecision = mg.alignPrecision();

    for( const auto &ask: mg.asks )
    {
        //auto tmp = ask.price;
        //tmp.precisionExpandTo(maxPricePrecision);
        std::string tmpStr = ask.price.to_string(maxPricePrecision);

        std::size_t w = tmpStr.size();
        if (maxPriceWidth<w)
            maxPriceWidth = w;
    }

    for( const auto &bid: mg.bids )
    {
        //auto tmp = bid.price;
        //tmp.precisionExpandTo(maxPricePrecision);
        std::string tmpStr = bid.price.to_string(maxPricePrecision);

        std::size_t w = tmpStr.size();
        if (maxPriceWidth<w)
            maxPriceWidth = w;
    }

    //------------------------------

    std::size_t priceWidth = 8 + 2*priceFieldWidth;
    std::size_t quantWidth = 6;

    std::string strEmptyPrice = std::string( priceWidth, ' ' );
    std::string strEmptyQuant = std::string( quantWidth, ' ' );
    std::string strSep        = std::string( sepWidth  , ' ' );
    

    //------------------------------

    for( auto ask: mg.asks )
    {
        s << strEmptyQuant ;
        s << strSep;
        s << marketGlassFormatHelperForPrice( ask.price, maxPricePrecision, maxPriceWidth );
        s << strSep;
        s << marketGlassFormatHelperForQuantity(ask.quantity, quantWidth );
        s << endl;
    }

    std::string baSep = strEmptyQuant + strSep + strEmptyPrice + strSep + strEmptyQuant;
    std::size_t baSepWidth = baSep.size();
    s << std::string(baSepWidth, '-') << endl;

    for( auto bid: mg.bids )
    {
        s << marketGlassFormatHelperForQuantity(bid.quantity, quantWidth );
        s << strSep;
        s << marketGlassFormatHelperForPrice( bid.price, maxPricePrecision, maxPriceWidth );
        s << strSep;
        s << strEmptyQuant ;
        s << endl;
    }

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

