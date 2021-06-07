#pragma once

#include <iostream>
#include <fstream>
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


#include "invest_openapi/config_helpers.h"
#include "invest_openapi/api_config.h"
#include "invest_openapi/auth_config.h"
#include "invest_openapi/currencies_config.h"
#include "invest_openapi/balance_config.h"

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




//----------------------------------------------------------------------------
namespace tkf_utils
{



//----------------------------------------------------------------------------
struct OrderParams
{
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


    QString     figi;
    QString     ticker;

    marty::Decimal        orderSize ; // in pieces, not in lots. Need to convert to lots number
    marty::Decimal        orderPrice; // Need to be adjusted to valid price with priceIncrement


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

     QString getOrderTypeString() const
     {
         if (orderType==orderTypeLimit)
             return QString("LIMIT");
         if (orderType==orderTypeMarket)
             return QString("MARKET");
         return QString("AUTO");
     }




}; // struct OrderParams

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------



inline
int parseOrderParams( const std::vector<std::string> &paramsVec
                     , const invest_openapi::DatabaseDictionaries &dicts
                     , OrderParams &orderParams
                     )
{
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

    orderParams.orderSize = marty::Decimal(*pit);
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

    orderParams.orderPrice = marty::Decimal(strPrice);

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

} // namespace tkf_utils


