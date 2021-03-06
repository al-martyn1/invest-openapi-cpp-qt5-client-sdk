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
 * CurrencyPosition.h
 *
 * 
 */

#ifndef CurrencyPosition_H
#define CurrencyPosition_H

#include <QJsonObject>

#include "Currency.h"

#include "Enum.h"
#include "Object.h"

#include "../../marty_decimal.h"


namespace OpenAPI {

class CurrencyPosition : public Object {
public:
    CurrencyPosition();
    CurrencyPosition(QString json);
    ~CurrencyPosition() override;

    QString asJson() const override;
    QJsonObject asJsonObject() const override;
    void fromJsonObject(QJsonObject json) override;
    void fromJson(QString jsonString) override;

    Currency getCurrency() const;
    void setCurrency(const Currency &currency);
    bool is_currency_Set() const;
    bool is_currency_Valid() const;

    marty::Decimal getBalance() const;
    void setBalance(const marty::Decimal &balance);
    bool is_balance_Set() const;
    bool is_balance_Valid() const;

    marty::Decimal getBlocked() const;
    void setBlocked(const marty::Decimal &blocked);
    bool is_blocked_Set() const;
    bool is_blocked_Valid() const;

    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    Currency currency;
    bool m_currency_isSet;
    bool m_currency_isValid;

    marty::Decimal balance;
    bool m_balance_isSet;
    bool m_balance_isValid;

    marty::Decimal blocked;
    bool m_blocked_isSet;
    bool m_blocked_isValid;
};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::CurrencyPosition)

#endif // CurrencyPosition_H
