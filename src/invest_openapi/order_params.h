#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <exception>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <optional>
#include <atomic>
#include <memory>

#include <QCoreApplication>
#include <QString>


#include "config_helpers.h"
#include "api_config.h"
#include "auth_config.h"
#include "currencies_config.h"
#include "balance_config.h"

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

#include "marty_decimal.h"




//----------------------------------------------------------------------------
namespace invest_openapi
{



//----------------------------------------------------------------------------
struct OrderParams
{

    typedef marty::Decimal   Decimal ;

    enum OrderOperationType
    {
        operationTypeBuy,
        operationTypeSell
    };

    enum OrderType
    {
        orderTypeAuto,
        orderTypeLimit,
        orderTypeMarket
    };

    /*
    OperationType.h
    enum class eOperationType {
        INVALID_VALUE_OPENAPI_GENERATED = 0,
        BUY, 
        SELL
    };
    */

    OrderOperationType    orderOperationType;
    OrderType             orderType;


    QString               figi;
    QString               ticker;

    Decimal               orderSize ; // in pieces, not in lots. Need to convert to lots number
    Decimal               orderPrice; // Need to be adjusted to valid price with priceIncrement

    // Decimal               instrumentPrice; // Для простоты, чтобы не высчитывать заново, сохраняем тут


/*
     COMMAND     '+' - покупка
                 '-' - продажа
   
     !           Маркер, что продаём по рынку, вне зависимости от величины спреда
                 (он игнорируется, если цена явно задана)

     ID          FIGI/TICKER - обычно TICKER

     SIZE        Размер заявки в штуках. Пересчёт в лоты производим автоматом. Оперируем 
                 всегда штуками. Если в целый лот не влезает, обрезаем. Но если 0 - то хоть 
                 один лот выставляем

     PRICE       Цена. Округляем по шагу, при продаже - вверх, при покупке - вниз.
                 Если цена задана, то заявка всегда лимитная.
*/

     bool isOrderTypeAuto()   const  { return orderType == orderTypeAuto; }
     bool isOrderTypeLimit()  const  { return orderType == orderTypeLimit; }
     bool isOrderTypeMarket() const  { return orderType == orderTypeMarket; }

     bool isSellOperation() const    { return orderOperationType==operationTypeSell; }


     OpenAPI::OrderType getOpenApiOrderType() const
     {
         if (isOrderTypeLimit())
             return OpenAPI::OrderType("LIMIT");

         return OpenAPI::OrderType("MARKET");
     }


     OpenAPI::OperationType getOpenApiOperationType() const
     {
         if (isSellOperation())
             return OpenAPI::OperationType("SELL");
         return OpenAPI::OperationType("BUY");
     }

     QString getOperationTypeString() const
     {
         if (isSellOperation())
             return QString("SELL");
         return QString("BUY");
     }

     QString getOperationTypeSignString() const
     {
         if (isSellOperation())
             return QString("-");
         return QString("+");
     }

     QString getOrderTypeString() const
     {
         if (orderType==orderTypeLimit)
             return QString("LIMIT");
         if (orderType==orderTypeMarket)
             return QString("MARKET");
         return QString("AUTO");
     }


     template< typename LotSizeType >
     unsigned calcNumLots( LotSizeType lotSize ) const
     {
         unsigned uLotSize   = unsigned(lotSize);
         unsigned uOrderSize = unsigned(orderSize);

         if (uLotSize==0)
             throw std::runtime_error("invest_openapi::OrderParams::calcNumLots: lot size can't be zero");

         unsigned numLots = uOrderSize / uLotSize;

         if (!numLots)
             numLots = 1;

         return numLots;
     }


     OrderParams getAdjusted( int spreadPoints
                            , const Decimal &priceIncrement
                            , const Decimal &asksMinPrice
                            , const Decimal &bidsMaxPrice
                            ) const
     {
         OrderParams orderParams = *this;

         if (orderParams.isOrderTypeAuto() && spreadPoints<=1)
         {
             orderParams.orderType = orderTypeMarket;
         }
         else if (orderParams.orderType==orderTypeMarket)
         {
             // Do nothing
         }
         else // orderParams.orderType==orderTypeLimit or (Auto && SpreadPoints>1)
         {
             orderParams.orderType = orderTypeLimit;

             if (orderParams.orderPrice==0) // Цену лимитной заявки подбираем автоматом так, чтобы продалось побыстрее, но по не самой плохой цене
             {
                 if (orderParams.isSellOperation()) // При продаже, автоматически вычисляя цену, делаем её на один пункт ниже уже выставленной (минимальный аск-шаг)
                     orderParams.orderPrice = asksMinPrice - priceIncrement;
                 else                               // При покупке, автоматически вычисляя цену, делаем её на один пункт выше уже выставленной (максимальный бид+шаг)
                     orderParams.orderPrice = bidsMaxPrice + priceIncrement;
             }
         }


         if (orderParams.orderPrice!=0)
         {
             Decimal finalPriceMod        = orderParams.orderPrice.mod_helper( priceIncrement );
             Decimal finalPriceCandidate  = finalPriceMod * priceIncrement;
             Decimal deltaPrice           = orderParams.orderPrice - finalPriceCandidate;
            
             if (deltaPrice!=0)
             {
                 if (orderParams.isSellOperation())
                     orderParams.orderPrice = finalPriceCandidate + priceIncrement;
                 else
                     orderParams.orderPrice = finalPriceCandidate;
             }
         }

         return orderParams;

     }

     //! Return true if price is good (or order request is not limit), or false overwise
     bool isLimitPriceCorrect(const Decimal &asksMinPrice, const Decimal &bidsMaxPrice) const
     {
         if (orderType!=orderTypeLimit)
             return true;

         if (orderOperationType==operationTypeBuy)
         {
             // Если покупаем по лимитной заявке, то цена быть ниже текущей, иначе, возможно, перепутано действие - покупка/продажа

             if (orderPrice >= asksMinPrice )
             {
                 // Ошибка в цене и/или типе операции
                 return false;
             }
         }
         else
         {
             // Если продаём по лимитной заявке, то цена быть выше текущей, иначе, возможно, перепутано действие - продажа/покупка
         
             if (orderPrice <= bidsMaxPrice )
             {
                 // Ошибка в цене и/или типе операции
                 return false;
             }
         }
                            
         return true;
     }


     QString toQString() const
     {
         QString res;

         res += getOperationTypeSignString();
         res += " ";

         if (isOrderTypeMarket()) // forced market
         {
             res += "! ";
         }

         res += ticker; // figi;
         res += " ";

         res += QString::fromStdString( orderSize.toString() );

         if (orderPrice!=0)
         {
             res += " ";
             res += QString::fromStdString( orderPrice.toString() );
         }

         return res;
     
     }


     std::string toString () const      { return toQString().toStdString(); }
     std::string to_string() const      { return toString(); }




}; // struct OrderParams

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
bool isOrderParamsStringSpecialChar( char ch )
{
    switch(ch)
    {
        case '+': 
        case '-': 
        case '!': 
             return true;

        default:
             return false;
    }
}

//----------------------------------------------------------------------------
inline 
bool prepareOrderIsIdString( const std::string &str )
{
    auto isNumericString = []( const std::string &str ) -> bool
                         {
                             for( auto ch : str)
                             {
                                 if (ch=='.' || ch==',')
                                     continue; 
                                 if (ch>='0' && ch<='9')
                                     continue; 
                                 return false;
                             }

                             return true;
                         };

    if (isNumericString(str))
        return false;

    auto isAlphaChar = []( char ch ) -> bool
                         {
                             if ( (ch>='a' && ch<='z') || (ch>='A' && ch<='Z') )
                                 return true;
                             return false;
                         };

    auto isDigitChar = []( char ch ) -> bool
                         {
                             if ( (ch>='0' && ch<='9') )
                                 return true;
                             return false;
                         };

    for( auto ch : str)
    {
        if (ch!='_' && !isAlphaChar(ch) && !isDigitChar(ch) )
            return false;
    }

    return true;

}


//----------------------------------------------------------------------------



inline 
std::string prepareOrderParamsString( const std::string &str )
{
    std::string resStr;

    for( auto ch : str )
    {
        if (!resStr.empty() && isOrderParamsStringSpecialChar(resStr.back()) && ch!=' ')
        {
            resStr.append(1, ' ');
        }

        resStr.append(1, ch );
    }

    return resStr;
}

//----------------------------------------------------------------------------
inline 
std::vector<std::string> splitOrderParamsString( const std::string &str )
{
    auto isEmptyString = []( const std::string &str ) -> bool
                         {
                             for( auto ch : str)
                             {
                                 if (ch!=' ')
                                     return false;
                             }

                             return true;
                         };

    std::stringstream iss(str);

    std::string tmp;

    std::vector<std::string> resVec;

    while(std::getline(iss,tmp,' '))
    {
        if (!isEmptyString(tmp))
            resVec.push_back(tmp);
    }

    return resVec;
}

//----------------------------------------------------------------------------
inline
std::string mergeOrderParamsString( const std::vector<std::string> &paramsVec )
{
    std::string resStr;

    for( const auto &s : paramsVec)
    {
        if (!resStr.empty())
        {
            if (!s.empty() && s[0]!=' ')
                resStr.append(1, ' ');
        }

        resStr.append(s);
    }

    return resStr;
}

//----------------------------------------------------------------------------
inline
std::string prepareOrderParams( const std::string &str )
{
    return prepareOrderParamsString( str );
}

//----------------------------------------------------------------------------
inline
std::string prepareOrderParams( const std::vector<std::string> &paramsVec )
{
    return prepareOrderParamsString( mergeOrderParamsString(paramsVec) );
}

//----------------------------------------------------------------------------
inline
int parseOrderParams( std::vector<std::string> paramsVec
                     , const invest_openapi::DatabaseDictionaries &dicts
                     , OrderParams &orderParams
                     )
{
    paramsVec = splitOrderParamsString( prepareOrderParams(paramsVec) );

    int errIdx = -1;

    std::vector<std::string>::const_iterator pit = paramsVec.begin();
    if (pit==paramsVec.end())
        return errIdx;

    QString opStr = QString::fromStdString(*pit).toUpper();

    if (opStr=="+" || opStr=="BUY")
        orderParams.orderOperationType = OrderParams::operationTypeBuy;
    else if (opStr=="-" || opStr=="SELL")
        orderParams.orderOperationType = OrderParams::operationTypeSell;
    else
        return errIdx;


    ++pit; --errIdx;
    if (pit==paramsVec.end())
        return errIdx;

    orderParams.orderType = OrderParams::orderTypeAuto; // orderTypeLimit

    if (*pit=="!") // Force market order
    {
        orderParams.orderType = OrderParams::orderTypeMarket;
        ++pit; --errIdx;
        if (pit==paramsVec.end())
            return errIdx;
    }

    QString idStr = QString::fromStdString(*pit); // .toUpper();
    orderParams.figi = dicts.findFigiByAnyIdString(idStr);

    if (orderParams.figi.isEmpty())
    {
        return errIdx;
    }

    orderParams.ticker = dicts.getTickerByFigiChecked(orderParams.figi);

    ++pit; --errIdx;
    if (pit==paramsVec.end())
        return errIdx;

    orderParams.orderSize = Decimal(*pit);
    if (orderParams.orderSize==0)
        return errIdx;

    orderParams.orderPrice = 0;

    ++pit; --errIdx;
    if (pit==paramsVec.end())
        return 0;

    std::string strPrice = *pit;
    std::string::size_type commaPos = strPrice.find(',');
    if (commaPos!=std::string::npos)
    {
        strPrice.replace(commaPos, 1, ".");
    }

    orderParams.orderPrice = Decimal(strPrice);

    if (orderParams.orderPrice==0)
        return errIdx;

    if (orderParams.orderType == OrderParams::orderTypeMarket)
        return errIdx;
    

    // Exact price can be taken for limit orders only
    orderParams.orderType = OrderParams::orderTypeLimit;

    return 0;
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------

} // namespace invest_openapi


