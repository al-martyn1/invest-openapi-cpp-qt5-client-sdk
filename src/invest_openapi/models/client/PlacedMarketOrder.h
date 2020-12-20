/**
 * OpenAPI
 * tinkoff.ru/invest OpenAPI.
 *
 * The version of the OpenAPI document: 1.0.0
 * Contact: n.v.melnikov@tinkoff.ru
 *
 * NOTE: This class is auto generated by OpenAPI Generator (https://openapi-generator.tech).
 * https://openapi-generator.tech
 * Do not edit the class manually.
 */

/*
 * PlacedMarketOrder.h
 *
 * 
 */

#ifndef PlacedMarketOrder_H
#define PlacedMarketOrder_H

#include <QJsonObject>

#include "MoneyAmount.h"
#include "OperationType.h"
#include "OrderStatus.h"
#include <QString>

#include "Enum.h"
#include "Object.h"

namespace OpenAPI {

class PlacedMarketOrder : public Object {
public:
    PlacedMarketOrder();
    PlacedMarketOrder(QString json);
    ~PlacedMarketOrder() override;

    QString asJson() const override;
    QJsonObject asJsonObject() const override;
    void fromJsonObject(QJsonObject json) override;
    void fromJson(QString jsonString) override;

    QString getOrderId() const;
    void setOrderId(const QString &order_id);
    bool is_order_id_Set() const;
    bool is_order_id_Valid() const;

    OperationType getOperation() const;
    void setOperation(const OperationType &operation);
    bool is_operation_Set() const;
    bool is_operation_Valid() const;

    OrderStatus getStatus() const;
    void setStatus(const OrderStatus &status);
    bool is_status_Set() const;
    bool is_status_Valid() const;

    QString getRejectReason() const;
    void setRejectReason(const QString &reject_reason);
    bool is_reject_reason_Set() const;
    bool is_reject_reason_Valid() const;

    QString getMessage() const;
    void setMessage(const QString &message);
    bool is_message_Set() const;
    bool is_message_Valid() const;

    qint32 getRequestedLots() const;
    void setRequestedLots(const qint32 &requested_lots);
    bool is_requested_lots_Set() const;
    bool is_requested_lots_Valid() const;

    qint32 getExecutedLots() const;
    void setExecutedLots(const qint32 &executed_lots);
    bool is_executed_lots_Set() const;
    bool is_executed_lots_Valid() const;

    MoneyAmount getCommission() const;
    void setCommission(const MoneyAmount &commission);
    bool is_commission_Set() const;
    bool is_commission_Valid() const;

    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    QString order_id;
    bool m_order_id_isSet;
    bool m_order_id_isValid;

    OperationType operation;
    bool m_operation_isSet;
    bool m_operation_isValid;

    OrderStatus status;
    bool m_status_isSet;
    bool m_status_isValid;

    QString reject_reason;
    bool m_reject_reason_isSet;
    bool m_reject_reason_isValid;

    QString message;
    bool m_message_isSet;
    bool m_message_isValid;

    qint32 requested_lots;
    bool m_requested_lots_isSet;
    bool m_requested_lots_isValid;

    qint32 executed_lots;
    bool m_executed_lots_isSet;
    bool m_executed_lots_isValid;

    MoneyAmount commission;
    bool m_commission_isSet;
    bool m_commission_isValid;
};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::PlacedMarketOrder)

#endif // PlacedMarketOrder_H