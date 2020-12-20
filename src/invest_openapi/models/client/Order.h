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
 * Order.h
 *
 * 
 */

#ifndef Order_H
#define Order_H

#include <QJsonObject>

#include "OperationType.h"
#include "OrderStatus.h"
#include "OrderType.h"
#include <QString>

#include "Enum.h"
#include "Object.h"

namespace OpenAPI {

class Order : public Object {
public:
    Order();
    Order(QString json);
    ~Order() override;

    QString asJson() const override;
    QJsonObject asJsonObject() const override;
    void fromJsonObject(QJsonObject json) override;
    void fromJson(QString jsonString) override;

    QString getOrderId() const;
    void setOrderId(const QString &order_id);
    bool is_order_id_Set() const;
    bool is_order_id_Valid() const;

    QString getFigi() const;
    void setFigi(const QString &figi);
    bool is_figi_Set() const;
    bool is_figi_Valid() const;

    OperationType getOperation() const;
    void setOperation(const OperationType &operation);
    bool is_operation_Set() const;
    bool is_operation_Valid() const;

    OrderStatus getStatus() const;
    void setStatus(const OrderStatus &status);
    bool is_status_Set() const;
    bool is_status_Valid() const;

    qint32 getRequestedLots() const;
    void setRequestedLots(const qint32 &requested_lots);
    bool is_requested_lots_Set() const;
    bool is_requested_lots_Valid() const;

    qint32 getExecutedLots() const;
    void setExecutedLots(const qint32 &executed_lots);
    bool is_executed_lots_Set() const;
    bool is_executed_lots_Valid() const;

    OrderType getType() const;
    void setType(const OrderType &type);
    bool is_type_Set() const;
    bool is_type_Valid() const;

    double getPrice() const;
    void setPrice(const double &price);
    bool is_price_Set() const;
    bool is_price_Valid() const;

    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    QString order_id;
    bool m_order_id_isSet;
    bool m_order_id_isValid;

    QString figi;
    bool m_figi_isSet;
    bool m_figi_isValid;

    OperationType operation;
    bool m_operation_isSet;
    bool m_operation_isValid;

    OrderStatus status;
    bool m_status_isSet;
    bool m_status_isValid;

    qint32 requested_lots;
    bool m_requested_lots_isSet;
    bool m_requested_lots_isValid;

    qint32 executed_lots;
    bool m_executed_lots_isSet;
    bool m_executed_lots_isValid;

    OrderType type;
    bool m_type_isSet;
    bool m_type_isValid;

    double price;
    bool m_price_isSet;
    bool m_price_isValid;
};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::Order)

#endif // Order_H