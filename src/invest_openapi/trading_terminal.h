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
#include "invest_openapi/term_helpers.h"


//----------------------------------------------------------------------------
namespace invest_openapi {


namespace trading_terminal {



//----------------------------------------------------------------------------
struct InstrumentInfoLineData
{
    typedef   marty::Decimal  Decimal;

    static const std::size_t  maxLastOperations = 3;

    QString   ticker;

    bool      isTraded; // Торгуется ли сейчас?

    Decimal   avgPrice; // Средняя цена покупки - брать из портфолио
    unsigned  quantity; // Количество бумаг в портфеле - брать из портфолио

    Decimal   priceIncrement; // 

    Decimal   curPrice    ; // Текущая цена - берём из стакана, если не торгуется - показываем последнюю. Или не показывать?
    Decimal   bestAskPrice; // берём из стакана
    Decimal   bestBidPrice; // берём из стакана
    Decimal   spread      ; // берём из стакана
    unsigned  spreadPoints; // берём из стакана

    Decimal   lastSellPrice   ; // Цена (средняя) последней продажи. Берём из операций по инструменту
    unsigned  lastSellQuantity; // Количество executed акций (не лотов). Берём из операций по инструменту

    Decimal   lastBuyPrice    ; // Цена (средняя) последней покупки. Берём из операций по инструменту
    unsigned  lastBuyQuantity ; // Количество executed акций/бумаг (не лотов). Берём из операций по инструменту


    Decimal   minAskPrice; // Цена продажи (в заявке), если заявка установлена, или 0 (выводим прочерк). Берём из orders минимальную по цене заявку на продажу
    unsigned  askQuantity; // Количество акций/бумаг в заявке. Берём из orders минимальную по цене заявку на продажу

    Decimal   maxBidPrice; // Цена покупки (в заявке), если заявка установлена, или 0 (выводим прочерк). Берём из orders максимальную по цене заявку на покупку
    unsigned  bidQuantity; // Количество акций/бумаг в заявке. Берём из orders максимальную по цене заявку на покупку

    std::vector<OpenAPI::Order>    sellOrders;
    std::vector<OpenAPI::Order>    buyOrders ;

    std::vector< tkf::Operation >  lastSellOperations;
    std::vector< tkf::Operation >  lastBuyOperations ;


    void init( const tkf::DatabaseDictionaries &dicts, QString figi )
    {
        if (!ticker.isEmpty())
            return;

        figi     = dicts.findFigiByAnyIdString (figi);
        ticker   = dicts.getTickerByFigiChecked(figi);

        isTraded = false;

        avgPrice = Decimal(0);
        quantity = 0;

        priceIncrement = Decimal(0);
        curPrice       = Decimal(0);
        bestAskPrice   = Decimal(0);
        bestBidPrice   = Decimal(0);

        spread         = Decimal(0);
        spreadPoints   = Decimal(0);


        lastSellPrice    = Decimal(0);
        lastSellQuantity = 0;

        lastBuyPrice     = Decimal(0);
        lastBuyQuantity  = 0;         


        minAskPrice      = Decimal(0);
        askQuantity      = 0;         

        maxBidPrice      = Decimal(0);
        bidQuantity      = 0;         

    }


    void update( const tkf::MarketInstrumentState &marketInstrumentState )
    {
        isTraded = marketInstrumentState.isTradeStatusNormalTrading();
    }

    void update()


}; // struct InstrumentInfoLineData




/*
    tkf::DatabaseDictionaries dicts = tkf::DatabaseDictionaries(pMainDbMan);

    std::map< QString, tkf::MarketInstrumentState >                        instrumentStates;
    std::map< QString, tkf::MarketGlass           >                        instrumentGlasses;
    std::map< QString, std::vector< tkf::Operation > >                     instrumentOperations;
    std::map< QString, std::vector<OpenAPI::Order> >                       activeOrders;
*/



//----------------------------------------------------------------------------


} // namespace trading_terminal

} // namespace invest_openapi

//----------------------------------------------------------------------------

