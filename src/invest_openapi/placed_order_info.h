#pragma once


#include "models.h"
#include "streaming_models/StreamingOrderbookItem.h"
#include "qt_helpers.h"
#include "qt_time_helpers.h"
#include "../cpp/cpp.h"

#include "market_math.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <map>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <utility>
#include <iterator>



//----------------------------------------------------------------------------
namespace invest_openapi
{




//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
struct PlacedOrderInfo
{
    // m_order_id_isValid && m_operation_isValid && m_status_isValid && m_requested_lots_isValid && m_executed_lots_isValid && true;

    QString                   trackingId    ; // Usualy filled from response
    QString                   status        ; // Usualy filled from response


    QString                   orderId       ; // getOrderId() const;
    OpenAPI::OperationType    operationType ; // getOperation()
    OpenAPI::OrderStatus      orderStatus   ; // getStatus() const;
    unsigned                  requestedLots ; // qint32 getRequestedLots() const;
    unsigned                  executedLots  ; // qint32 getExecutedLots() const;
    OpenAPI::MoneyAmount      commission    ; // getCommission()


    // Optional
    QString                   rejectReason  ; // getRejectReason()
    QString                   message       ; // getMessage()


    bool isValid() const { return !orderId.isEmpty(); }


    static
    PlacedOrderInfo fromPlacedOrder( const OpenAPI::PlacedMarketOrder &o )
    {
        PlacedOrderInfo poRes;

        if (!o.isSet() || !o.isValid())
            return poRes;

        poRes.orderId        = o.getOrderId()   ;
        poRes.operationType  = o.getOperation() ;
        poRes.orderStatus    = o.getStatus()    ;
        poRes.requestedLots  = o.getRequestedLots();
        poRes.executedLots   = o.getExecutedLots() ;
        poRes.commission     = o.getCommission();

        poRes.rejectReason  = o.getRejectReason();
        poRes.message       = o.getMessage();

        return poRes;

    }


    static
    PlacedOrderInfo fromPlacedOrder( const OpenAPI::PlacedLimitOrder &o )
    {
        PlacedOrderInfo poRes;

        if (!o.isSet() || !o.isValid())
            return poRes;

        poRes.orderId        = o.getOrderId()   ;
        poRes.operationType  = o.getOperation() ;
        poRes.orderStatus    = o.getStatus()    ;
        poRes.requestedLots  = o.getRequestedLots();
        poRes.executedLots   = o.getExecutedLots() ;
        poRes.commission     = o.getCommission();

        poRes.rejectReason  = o.getRejectReason();
        poRes.message       = o.getMessage();

        return poRes;

    }


    static
    PlacedOrderInfo fromOrderResponse( const OpenAPI::MarketOrderResponse &response )
    {
        PlacedOrderInfo poRes;

        if (!response.isSet() || !response.isValid())
            return poRes;

        if (!response.is_payload_Set() || !response.is_payload_Valid())
            return poRes;

        poRes = fromPlacedOrder( response.getPayload() );

        poRes.trackingId = response.getTrackingId();
        poRes.status     = response.getStatus();

        return poRes;
    }


    static
    PlacedOrderInfo fromOrderResponse( const OpenAPI::LimitOrderResponse &response )
    {
        PlacedOrderInfo poRes;

        if (!response.isSet() || !response.isValid())
            return poRes;

        if (!response.is_payload_Set() || !response.is_payload_Valid())
            return poRes;

        poRes = fromPlacedOrder( response.getPayload() );

        poRes.trackingId = response.getTrackingId();
        poRes.status     = response.getStatus();

        return poRes;
    }




};
//----------------------------------------------------------------------------






//----------------------------------------------------------------------------

} // namespace invest_openapi

