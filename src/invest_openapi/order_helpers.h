#pragma once


#include <QCoreApplication>
#include <QString>
#include <QSettings>
#include <QTest>
#include <QDir>
#include <QList>
#include <QElapsedTimer>
#include <QtWebSockets>

#include <algorithm>
#include <iterator>
#include <ostream>
#include <iostream>
#include <iomanip>
#include <exception>
#include <stdexcept>

#include "models.h"
#include "invest_openapi_types.h"
#include "invest_openapi.h"
#include "ioa_ostream.h"
#include "openapi_completable_future.h"
#include "openapi_limits.h"
#include "cpp_helpers.h"


//----------------------------------------------------------------------------
namespace invest_openapi
{



//----------------------------------------------------------------------------
//! 
inline
void parseOrders( const QList<Order>                                &orders
                , std::map< QString, std::vector<OpenAPI::Order> >  &parseToMap
                )
{
    for( const auto &o : orders )
    {
        parseToMap[o.getFigi().toUpper()].push_back(o);
    }
}

//----------------------------------------------------------------------------
//! Return true if response is valid, and payload is also valid
inline
bool parseOrdersResponse( const OpenAPI::OrdersResponse                     &ordersResponse
                        , std::map< QString, std::vector<OpenAPI::Order> >  &parseToMap
                        )
{
    if (!ordersResponse.isSet() || !ordersResponse.isValid())
        return false;

    if (!ordersResponse.is_payload_Set() || !ordersResponse.is_payload_Valid())
        return false;

    parseOrders( ordersResponse.getPayload()
               , parseToMap
               );

    return true;
}

//----------------------------------------------------------------------------
inline // return true if no error, response is valid and parsed ok
bool processCompletedOrdersResponse( QSharedPointer< OpenApiCompletableFuture<OpenAPI::OrdersResponse> >   ordersResponse
                                   , std::map< QString, std::vector<OpenAPI::Order> >                      &parseToMap
                                   )
{
    if (!ordersResponse)
        throw("invest_openapi::processAwaitingOrdersResponse: ordersResponse is zero");

    //if (!ordersResponse->isFinished())
    //    return false;

    if (!ordersResponse->isResultValid())
        return false;

    return parseOrdersResponse( ordersResponse->result(), parseToMap );

}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
bool isOrderOperationTypeSell( const OpenAPI::Order &op )
{
    // QString operationTypeStr   = op.getOperationType().asJson().toUpper();
    // if (operationTypeStr!="SELL")
    //     return false;
    // return true;


    return op.getOperation().getValue() == OpenAPI::OperationType::eOperationType::SELL;
}

//----------------------------------------------------------------------------
inline
bool isOrderStatusActiveOrder( const OpenAPI::Order &op )
{
    auto statusValue = op.getStatus().getValue();
    return statusValue==OpenAPI::OrderStatus::eOrderStatus::NEW
        || statusValue==OpenAPI::OrderStatus::eOrderStatus::PARTIALLYFILL // Полагаю, это значит - частично исполнен
        ;
}

//----------------------------------------------------------------------------
inline
void sortOrdersByPriceAscending( std::vector< OpenAPI::Order > & ops )
{
    std::stable_sort( ops.begin(), ops.end()
                    , []( const OpenAPI::Order &op1, const OpenAPI::Order &op2 )
                      {
                          return op1.getPrice() < op2.getPrice();
                      }
                    );
}
//----------------------------------------------------------------------------

inline
void sortOrdersByPriceDescending( std::vector< OpenAPI::Order > & ops )
{
    std::stable_sort( ops.begin(), ops.end()
                    , []( const OpenAPI::Order &op1, const OpenAPI::Order &op2 )
                      {
                          return op1.getPrice() > op2.getPrice();
                      }
                    );
}

//----------------------------------------------------------------------------
inline
void sortOrdersByPrice( std::vector< OpenAPI::Order > & ops, SortType sortType )
{
    if (sortType==SortType::ascending)
        sortOrdersByPriceAscending(ops);
    else
        sortOrdersByPriceDescending(ops);
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
template< typename SrcIteratorType, typename InserterType > inline
void splitOrdersByOperationType( SrcIteratorType b, SrcIteratorType e
                               , InserterType inserterSell, InserterType inserterBuy
                               )
{
    for( ; b!=e; ++b )
    {
        if (isOrderOperationTypeSell( *b ) )
        {
            *inserterSell++ = *b;
        }
        else
        {
            *inserterBuy++ = *b;
        }
    }
}

//----------------------------------------------------------------------------
template< typename VectorType > inline
void splitOrdersByOperationType( const VectorType   &src
                               , VectorType &sells, VectorType &buys
                               , bool bAppend = false
                               )
{
    if (!bAppend)
    {
        sells.clear();
        buys .clear();
    }

    splitOrdersByOperationType( src.begin(), src.end(), std::back_inserter(sells), std::back_inserter(buys) );
}

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------




} // namespace invest_openapi

//----------------------------------------------------------------------------

