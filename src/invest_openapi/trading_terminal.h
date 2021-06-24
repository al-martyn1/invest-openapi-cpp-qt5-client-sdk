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

#include "invest_openapi/invest_openapi.h"
#include "invest_openapi/factory.h"
#include "invest_openapi/openapi_completable_future.h"

#include "invest_openapi/database_config.h"
#include "invest_openapi/database_manager.h"
#include "invest_openapi/qt_time_helpers.h"

#include "invest_openapi/db_utils.h"
#include "invest_openapi/ioa_utils.h"
#include "invest_openapi/ioa_ostream.h"
#include "invest_openapi/ioa_db_dictionaries.h"

#include "invest_openapi/openapi_limits.h"

#include "invest_openapi/market_glass.h"
#include "invest_openapi/market_instrument_state.h"

#include "invest_openapi/operation_helpers.h"
#include "invest_openapi/order_helpers.h"

#include "invest_openapi/format_helpers.h"
#include "invest_openapi/terminal_helpers.h"


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
            curPrice     = Decimal(0)
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

        tkf::splitOrdersByOperationType( orders, sellOrders, buyOrders, false /* bAppend */ );

        tkf::sortOrdersByPrice( sellOrders, tkf::SortType::ascending  );
        tkf::sortOrdersByPrice( buyOrders , tkf::SortType::descending );

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

    static
    std::string format_caption( const FieldFormat &ff )
    {
        return format_field<std::string>( fmt.leftSpace, fmt.rightSpace, fmt.fieldWidth, fmt.captionAlignment, caption.toStdString() );
    }

    std::string format_field( const FieldFormat &ff ) const
    {
        QString Id = id.toUpper();

        if (Id=="TICKER")
        {
        }
        else if (Id=="STATE")
        {
        }
        else if (Id=="PRICE_INC")
        {
        }
        else if (Id=="LOT_SIZE")
        {
        }
        else if (Id=="PAID_PRICE")
        {
        }
        else if (Id=="QUANTITY")
        {
        }
        else if (Id=="CUR_PRICE")
        {
        }
        else if (Id=="BEST_BID")
        {
        }
        else if (Id=="BEST_ASK")
        {
        }
        else if (Id=="SPREAD_POINTS")
        {
        }
        else if (Id=="LAST_BUY_PRICE")
        {
        }
        else if (Id=="LAST_BUY_QUANTITY")
        {
        }
        else if (Id=="LAST_SELL_PRICE")
        {
        }
        else if (Id=="LAST_SELL_QUANTITY")
        {
        }
        else if (Id=="MAX_BID_PRICE")
        {
        }
        else if (Id=="MAX_BID_QUANTITY")
        {
        }
        else if (Id=="MIN_ASK_PRICE")
        {
        }
        else if (Id=="MIN_ASK_QUANTITY")
        {
        }
        // else if (Id=="")
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

