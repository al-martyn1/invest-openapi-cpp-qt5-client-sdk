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

        return res;

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

        return res;

    }


                    QString getTrackingId() const;
                    void setTrackingId(const QString &tracking_id);
                    bool is_tracking_id_Set() const;
                    bool is_tracking_id_Valid() const;
                
                    QString getStatus() const;




};
//----------------------------------------------------------------------------






//----------------------------------------------------------------------------

} // namespace invest_openapi

