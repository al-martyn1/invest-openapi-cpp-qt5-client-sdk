#pragma once


#include <iostream>
#include <ostream>
#include <exception>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <optional>
#include <atomic>
#include <deque>

#include <QString>
#include <QTest>
#include <QElapsedTimer>

#include "invest_openapi.h"
#include "factory.h"
#include "openapi_completable_future.h"

#include "database_config.h"
#include "database_manager.h"
#include "qt_time_helpers.h"

#include "db_utils.h"
#include "ioa_utils.h"
#include "ioa_ostream.h"
#include "ioa_db_dictionaries.h"

#include "openapi_limits.h"

#include "market_glass.h"
#include "market_instrument_state.h"

#include "operation_helpers.h"
#include "order_helpers.h"

#include "format_helpers.h"
#include "terminal_helpers.h"


//----------------------------------------------------------------------------
namespace invest_openapi {


namespace trading_terminal {



//----------------------------------------------------------------------------
struct InstrumentInfoLineData
{
    typedef   marty::Decimal  Decimal;

    static const std::size_t  maxLastOperations = 3;


    QString   ticker;


    bool      isTraded; //!< Торгуется ли сейчас? (From streaming MarketInstrumentState)

    Decimal   priceIncrement; //!< From streaming MarketInstrumentState
    unsigned  lotSize       ; //!< From streaming MarketInstrumentState


    Decimal   avgPrice; //!< Средняя цена покупки - брать из портфолио
    unsigned  quantity; //!< Количество бумаг в портфеле - брать из портфолио


    Decimal   curPrice    ; //!< Текущая цена - берём из стакана, если не торгуется - показываем последнюю. Или не показывать?
    Decimal   bestAskPrice; //!< берём из стакана
    Decimal   bestBidPrice; //!< берём из стакана
    Decimal   spread      ; //!< берём из стакана
    int       spreadPoints; //!< берём из стакана


    Decimal   lastSellPrice   ; //!< Цена (средняя) последней продажи. Берём из операций по инструменту
    unsigned  lastSellQuantity; //!< Количество executed акций (не лотов). Берём из операций по инструменту

    Decimal   lastBuyPrice    ; //!< Цена (средняя) последней покупки. Берём из операций по инструменту
    unsigned  lastBuyQuantity ; //!< Количество executed акций/бумаг (не лотов). Берём из операций по инструменту


    Decimal   minAskPrice; // Цена продажи (в заявке), если заявка установлена, или 0 (выводим прочерк). Берём из orders минимальную по цене заявку на продажу
    unsigned  minAskQuantity; // Количество акций/бумаг в заявке. Берём из orders минимальную по цене заявку на продажу

    Decimal   maxBidPrice; // Цена покупки (в заявке), если заявка установлена, или 0 (выводим прочерк). Берём из orders максимальную по цене заявку на покупку
    unsigned  maxBidQuantity; // Количество акций/бумаг в заявке. Берём из orders максимальную по цене заявку на покупку

    std::vector<OpenAPI::Order>    sellOrders;
    std::vector<OpenAPI::Order>    buyOrders ;

    std::vector< OpenAPI::Operation >  lastSellOperations;
    std::vector< OpenAPI::Operation >  lastBuyOperations ;


    void invalidateMarketStateFields()
    {
        isTraded       = false;
        priceIncrement = Decimal(0);
        lotSize        = 0;
    
    }

    void invalidateMarketGlassFields()
    {
        curPrice     = Decimal (0);
        bestAskPrice = Decimal (0);
        bestBidPrice = Decimal (0);
        spread       = Decimal (0);
        spreadPoints = unsigned(0);
    }


    void init( const DatabaseDictionaries &dicts, QString figi )
    {
        if (!ticker.isEmpty())
            return;

        figi     = dicts.findFigiByAnyIdString (figi);
        ticker   = dicts.getTickerByFigiChecked(figi);

        invalidateMarketStateFields();

        avgPrice = Decimal(0);
        quantity = 0;

        invalidateMarketGlassFields();


        lastSellPrice    = Decimal(0);
        lastSellQuantity = 0;

        lastBuyPrice     = Decimal(0);
        lastBuyQuantity  = 0;         


        minAskPrice      = Decimal(0);
        minAskQuantity   = 0;         

        maxBidPrice      = Decimal(0);
        maxBidQuantity   = 0;         

    }


    void update( const MarketInstrumentState &marketInstrumentState )
    {
        isTraded       = marketInstrumentState.isTradeStatusNormalTrading();
        priceIncrement = marketInstrumentState.priceIncrement;
        lotSize        = marketInstrumentState.lotSize       ;
    }

    void update( const MarketGlass &marketGlass )
    {
        if (priceIncrement==Decimal(0))
        {
            curPrice     = Decimal(0);
            spreadPoints = 0;
        }
        else
        {
            curPrice     = marketGlass.calcInstrumentPrice(priceIncrement);
            spreadPoints = marketGlass.getPriceSpreadPoints(priceIncrement);
        }
        
        bestAskPrice = marketGlass.getAskBestPrice();
        bestBidPrice = marketGlass.getBidBestPrice();
        spread       = marketGlass.getPriceSpread ();

    }

    //! \param operations must be sorted by date in descending order
    void update( const std::vector< OpenAPI::Operation > &operations )
    {
        getSomeOfFirstOperations( maxLastOperations, operations, lastSellOperations, lastBuyOperations, false /* bAppend */ ); 

        if (lastSellOperations.empty())
        {
            lastSellPrice    = Decimal(0);
            lastSellQuantity = 0;
        }
        else
        {
            lastSellPrice    = lastSellOperations[0].getPrice();
            lastSellQuantity = lastSellOperations[0].getQuantityExecuted();
        }

        if (lastBuyOperations.empty())
        {
            lastBuyPrice     = Decimal(0);
            lastBuyQuantity  = 0;
        }
        else
        {
            lastBuyPrice     = lastBuyOperations[0].getPrice();
            lastBuyQuantity  = lastBuyOperations[0].getQuantityExecuted();
        }

    }

    void update( const std::vector<OpenAPI::Order> &orders )
    {
        //getPrice()
        //getRequestedLots()

        splitOrdersByOperationType( orders, sellOrders, buyOrders, false /* bAppend */ );

        sortOrdersByPrice( sellOrders, SortType::ascending  );
        sortOrdersByPrice( buyOrders , SortType::descending );

        if (sellOrders.empty())
        {
            minAskPrice    = Decimal(0);
            minAskQuantity = 0;
        }
        else
        {
            minAskPrice    = sellOrders[0].getPrice();
            minAskQuantity = sellOrders[0].getRequestedLots();
        }

        if (buyOrders.empty())
        {
            maxBidPrice    = Decimal(0);
            maxBidQuantity = 0;
        }
        else
        {
            maxBidPrice    = buyOrders[0].getPrice();
            maxBidQuantity = buyOrders[0].getRequestedLots();
        }
    
    }

    std::string format_field( const FieldFormat &ff ) const
    {
        QString id = ff.id.toUpper();

        using invest_openapi::format_field;

        if (id=="TICKER")
        {
            return format_field( ff, ticker.toStdString() );
        }
        else if (id=="STATE")
        {
            return format_field( ff, isTraded ? "T" : "-" );
        }
        else if (id=="PRICE_INC")
        {
            if (priceIncrement==Decimal(0))
                return format_field( ff, "-" );

            return format_field( ff, priceIncrement );
        }
        else if (id=="LOT_SIZE")
        {
            if (lotSize==0)
                return format_field( ff, "-" );

            return format_field( ff, lotSize );
        }
        else if (id=="PAID_PRICE") //------------------------------------------
        {
            return format_field( ff, "-" );
        }
        else if (id=="QUANTITY")
        {
            return format_field( ff, "-" );
        }
        else if (id=="CUR_PRICE")
        {
            return format_field( ff, "-" );
        }
        else if (id=="BEST_BID")
        {
            return format_field( ff, "-" );
        }
        else if (id=="BEST_ASK")
        {
            return format_field( ff, "-" );
        }
        else if (id=="SPREAD_POINTS")
        {
            return format_field( ff, "-" );
        }
        else if (id=="LAST_BUY_PRICE")
        {
            return format_field( ff, "-" );
        }
        else if (id=="LAST_BUY_QUANTITY")
        {
            return format_field( ff, "-" );
        }
        else if (id=="LAST_SELL_PRICE")
        {
            return format_field( ff, "-" );
        }
        else if (id=="LAST_SELL_QUANTITY")
        {
            return format_field( ff, "-" );
        }
        else if (id=="MAX_BID_PRICE")
        {
            return format_field( ff, "-" );
        }
        else if (id=="MAX_BID_QUANTITY")
        {
            return format_field( ff, "-" );
        }
        else if (id=="MIN_ASK_PRICE")
        {
            return format_field( ff, "-" );
        }
        else if (id=="MIN_ASK_QUANTITY")
        {
            return format_field( ff, "-" );
        }
        // else if (id=="")
        // {
        // }


    }


}; // struct InstrumentInfoLineData






/*
    tkf::DatabaseDictionaries dicts = tkf::DatabaseDictionaries(pMainDbMan);

    std::map< QString, tkf::MarketInstrumentState >                        instrumentStates;      [+]
    std::map< QString, tkf::MarketGlass           >                        instrumentGlasses;     [+]
    std::map< QString, std::vector< tkf::Operation > >                     instrumentOperations;
    std::map< QString, std::vector<OpenAPI::Order> >                       activeOrders;
*/



//----------------------------------------------------------------------------


} // namespace trading_terminal

} // namespace invest_openapi

//----------------------------------------------------------------------------

