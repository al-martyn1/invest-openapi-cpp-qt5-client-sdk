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
#include "invest_openapi/ioa_db_dictionaries.h"


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

#include "invest_openapi/terminal_config.h"



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

    void update( const std::vector<OpenAPI::Order> &ordersParam )
    {
        //getPrice()
        //getRequestedLots()

        std::vector<OpenAPI::Order> orders;
        auto inserter = std::back_inserter(orders);
        for( const auto &o : ordersParam )
        {
           if (isOrderStatusActiveOrder(o))
               *inserter++ = o;
        }

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
            //return format_field( ff, "-" );
            return format_field( ff, "N/I" );
        }
        else if (id=="QUANTITY")
        {
            //return format_field( ff, "-" );
            return format_field( ff, "N/I" );
        }
        else if (id=="CUR_PRICE")
        {
            //return format_field( ff, "-" );
            return format_field( ff, "N/I" );
        }
        else if (id=="BEST_BID")
        {
            //return format_field( ff, "-" );
            return format_field( ff, "N/I" );
        }
        else if (id=="BEST_ASK")
        {
            //return format_field( ff, "-" );
            return format_field( ff, "N/I" );
        }
        else if (id=="SPREAD_POINTS")
        {
            //return format_field( ff, "-" );
            return format_field( ff, "N/I" );
        }
        else if (id=="LAST_BUY_PRICE")
        {
            //return format_field( ff, "-" );
            return format_field( ff, "N/I" );
        }
        else if (id=="LAST_BUY_QUANTITY")
        {
            //return format_field( ff, "-" );
            return format_field( ff, "N/I" );
        }
        else if (id=="LAST_SELL_PRICE")
        {
            //return format_field( ff, "-" );
            return format_field( ff, "N/I" );
        }
        else if (id=="LAST_SELL_QUANTITY")
        {
            //return format_field( ff, "-" );
            return format_field( ff, "N/I" );
        }
        else if (id=="MAX_BID_PRICE")
        {
            //return format_field( ff, "-" );
            return format_field( ff, "N/I" );
        }
        else if (id=="MAX_BID_QUANTITY")
        {
            //return format_field( ff, "-" );
            return format_field( ff, "N/I" );
        }
        else if (id=="MIN_ASK_PRICE")
        {
            //return format_field( ff, "-" );
            return format_field( ff, "N/I" );
        }
        else if (id=="MIN_ASK_QUANTITY")
        {
            //return format_field( ff, "-" );
            return format_field( ff, "N/I" );
        }
        // else if (id=="")
        // {
        // }

        return format_field( ff, "N/I" );

    }

    //------------------------------
    #define DECLARE_IOA_TRADING_TERMINAL_LINE_DATA_UPDATE_FUNCTION( dataType )                \
    static                                                                                    \
    void updateTerminalData( std::map< QString, InstrumentInfoLineData > &terminalData        \
                           , const DatabaseDictionaries &dicts, QString figi                  \
                           , const dataType &data                                             \
                           )                                                                  \
    {                                                                                         \
        figi           = figi.toUpper();                                                      \
        auto &lineData = terminalData[figi];                                                  \
                                                                                              \
        lineData.init(dicts, figi); /* Если новый элемент - инициализируется, если существующий - ничего не происходит */ \
                                                                                              \
        lineData.update(data);                                                                \
    }

    DECLARE_IOA_TRADING_TERMINAL_LINE_DATA_UPDATE_FUNCTION(MarketInstrumentState)
    DECLARE_IOA_TRADING_TERMINAL_LINE_DATA_UPDATE_FUNCTION(MarketGlass)
    DECLARE_IOA_TRADING_TERMINAL_LINE_DATA_UPDATE_FUNCTION(std::vector< OpenAPI::Operation >)
    DECLARE_IOA_TRADING_TERMINAL_LINE_DATA_UPDATE_FUNCTION(std::vector<OpenAPI::Order>)


}; // struct InstrumentInfoLineData

//----------------------------------------------------------------------------






//----------------------------------------------------------------------------
class TradingTerminalData
{

protected:

    QStringList                                                            instrumentList;
    std::map< QString, int >                                               instrumentNumbers;
    DatabaseDictionaries                                                  *pDicts;

    std::map< QString, MarketInstrumentState >                             instrumentStates;
    std::map< QString, MarketGlass           >                             instrumentGlasses;
    std::map< QString, std::vector< OpenAPI::Operation > >                 instrumentOperations;
    std::map< QString, std::vector< OpenAPI::Order > >                     activeOrders;

    QString                                                                statusStr;
    QDateTime                                                              statusChangedDateTime;

    QSharedPointer<TerminalConfig>                                         pTermConfig;

    std::map< QString, InstrumentInfoLineData >                            terminalLinesData;

    //------------------------------

    std::set< QString > updatedFigis;
    bool statusUpdated = false;

    //------------------------------

    template< typename ValType >
    bool updateVal( ValType &val, ValType newVal ) const //!< Возвращает true, если значение поменялось
    {
        bool res = false;

        if ( val!=newVal )
            res = true;

        val = newVal;

        return res;
    }


    //------------------------------
    bool updateFigiDataImpl( MarketInstrumentState &curData, const MarketInstrumentState &newData )
    {
        bool changed = false;

        // Тупо чекаем, что нужно почекать
        auto curTradeStatusValue = curData.tradeStatus.getValue();
        changed |= updateVal( curTradeStatusValue, newData.tradeStatus.getValue() );

        changed |= updateVal( curData.priceIncrement, newData.priceIncrement );
        changed |= updateVal( curData.lotSize       , newData.lotSize );

        curData = newData; // И присваиваем всё скопом

        return changed;
    }

    bool updateFigiDataImpl( MarketGlass &curData, const MarketGlass &newData )
    {
        curData = newData;
        return true;
    }

    bool updateFigiDataImpl( std::vector< OpenAPI::Operation > &curData, const std::vector< OpenAPI::Operation > &newData )
    {
        curData = newData; // UNDONE
        return true;
    }

    bool updateFigiDataImpl( std::vector< OpenAPI::Order > &curData, const std::vector< OpenAPI::Order > &newData )
    {
        curData = newData; // UNDONE
        return true;
    }



public:


    //------------------------------
    TradingTerminalData( DatabaseDictionaries *pd )
    : instrumentList       ()
    , instrumentNumbers    ()
    , pDicts               (pd)
    , instrumentStates     ()
    , instrumentGlasses    ()
    , instrumentOperations ()
    , activeOrders         ()
    , statusStr            ()
    , statusChangedDateTime()
    , pTermConfig          (0)
    , terminalLinesData    ()
    {
    }
   
    TradingTerminalData( DatabaseDictionaries *pd, QStringList il )
    : instrumentList       ()
    , instrumentNumbers    ()
    , pDicts               (pd)
    , instrumentStates     ()
    , instrumentGlasses    ()
    , instrumentOperations ()
    , activeOrders         ()
    , statusStr            ()
    , statusChangedDateTime()
    , pTermConfig          (0)
    , terminalLinesData    ()
    {
        setInstrumentList( il );
    }


    //------------------------------
    void setStatus( QString str )
    {
        statusStr             = str;
        statusChangedDateTime = QDateTime::currentDateTime();
        statusUpdated         = true;
    }

    QString getStatus() const           { return statusStr; }
    QDateTime getStatusDateTime() const { return statusChangedDateTime; }


    //------------------------------
    void setConfig( QSharedPointer<TerminalConfig> cfg )
    {
        pTermConfig = cfg;
    }


    //------------------------------
    bool addInstrument( QString figi )
    {
        figi = pDicts->findFigiByAnyIdString(figi);

        if (instrumentNumbers.find(figi)!=instrumentNumbers.end())
            return false;

        instrumentNumbers[figi] = (int)instrumentList.size();
        instrumentList.push_back(figi);

        auto &lineData = terminalLinesData[figi];
        
        lineData.init( *pDicts, figi );

        return true;
    }

    void setInstrumentList( QStringList il )
    {
         for( auto figi : il)
         {
             addInstrument(figi);
         }
    }

    //------------------------------

    QStringList::const_iterator instrumentListBegin() const { return instrumentList.begin(); }
    QStringList::const_iterator instrumentListEnd  () const { return instrumentList.end  (); }

    int getFigiCount( ) const { return instrumentNumbers.size(); }

    int getFigiIndex( QString figi ) const
    {
        figi.toUpper();

        std::map< QString, int >::const_iterator it = instrumentNumbers.find(figi);
        if (it==instrumentNumbers.end())
            return -1;

        return it->second;
    }

    QString getFigiByIndex( int idx ) const
    {
        std::size_t szIdx = (std::size_t)idx;
        if (szIdx>=(std::size_t)instrumentList.size())
            throw std::runtime_error("TradingTerminalData::getFigiByIndex: Instrument index is out of range");
        return instrumentList[szIdx];
    }

    //------------------------------
    bool isStatusChanged() const { return statusUpdated; }

    bool isFigiChanged()   const { return !updatedFigis.empty(); } 

    bool isFigiChanged( QString figi ) const
    {
        figi.toUpper();

        return updatedFigis.find(figi)!=updatedFigis.end();
    } 

    //------------------------------

    void clearChangedFlags()
    {
        statusUpdated = false;
        updatedFigis.clear();
    }


    //------------------------------
    std::size_t getMainViewColsCount() const
    {
        return pTermConfig->fieldsFormat.size();
    }


    std::vector< std::size_t > getMainViewColSizes() const
    {
        std::vector< std::size_t > res;

        std::size_t i = 0;
        for(; i!=pTermConfig->fieldsFormat.size(); ++i)
        {
            res.push_back( pTermConfig->fieldsFormat[i].getSummaryWidth() );
        }

        return res;
    }

    std::string formatMainViewColCaption( std::size_t colNo ) const
    {
        if (colNo>=pTermConfig->fieldsFormat.size())
            throw std::runtime_error( "TradingTerminalData::formatCaption: column index is out of range" );
        
        return format_field_caption( pTermConfig->fieldsFormat[colNo] );
    }


    // std::string format_field( const FieldFormat &ff ) const
    


    /*
    bool update( QString figi                                                                 
               , const MarketInstrumentState &data                                                         
               )                                                                              
    {                                                                                         
        figi           = figi.toUpper();                                                      
                                                                                              
        auto &figiData = instrumentStates [figi];                                                      
        if (updateFigiDataImpl(figiData,data))                                                
        {                                                                                     
            updatedFigis.insert(figi);                                                        
            auto &lineData = terminalLinesData[figi];                                         
            lineData.update(data);                                                            
                                                                                              
            return true;                                                                      
        }                                                                                     
                                                                                              
        return false;                                                                         
    }
    */

    #define DECLARE_IOA_TRADING_TERMINAL_DATA_UPDATE_FUNCTION( dataType, mapName )            \
    bool update( QString figi                                                                 \
               , const dataType &data                                                         \
               )                                                                              \
    {                                                                                         \
        figi           = figi.toUpper();                                                      \
                                                                                              \
        auto &figiData = mapName [figi];                                                      \
        if (updateFigiDataImpl(figiData,data))                                                \
        {                                                                                     \
            updatedFigis.insert(figi);                                                        \
            auto &lineData = terminalLinesData[figi];                                         \
            lineData.update(data);                                                            \
                                                                                              \
            return true;                                                                      \
        }                                                                                     \
                                                                                              \
        return false;                                                                         \
    }


    DECLARE_IOA_TRADING_TERMINAL_DATA_UPDATE_FUNCTION( MarketInstrumentState             , instrumentStates     )
    DECLARE_IOA_TRADING_TERMINAL_DATA_UPDATE_FUNCTION( MarketGlass                       , instrumentGlasses    )
    DECLARE_IOA_TRADING_TERMINAL_DATA_UPDATE_FUNCTION( std::vector< OpenAPI::Operation > , instrumentOperations )
    DECLARE_IOA_TRADING_TERMINAL_DATA_UPDATE_FUNCTION( std::vector< OpenAPI::Order >     , activeOrders         )

}; // class TradingTerminalData

//----------------------------------------------------------------------------







//----------------------------------------------------------------------------


} // namespace trading_terminal

} // namespace invest_openapi

//----------------------------------------------------------------------------

