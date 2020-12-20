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
 * Currency.h
 *
 * 
 */

#ifndef Currency_H
#define Currency_H

#include <QJsonObject>


#include "Enum.h"
#include "Object.h"

namespace OpenAPI {

class Currency : public Enum {
public:
    Currency();
    Currency(QString json);
    ~Currency() override;

    QString asJson() const override;
    QJsonValue asJsonValue() const override;
    void fromJsonValue(QJsonValue json) override;
    void fromJson(QString jsonString) override;

    enum class eCurrency {
        INVALID_VALUE_OPENAPI_GENERATED = 0,
        RUB, 
        USD, 
        EUR, 
        GBP, 
        HKD, 
        CHF, 
        JPY, 
        CNY, 
        TRY
    };
    Currency::eCurrency getValue() const;
    void setValue(const Currency::eCurrency& value);
    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    eCurrency m_value;
    bool m_value_isSet;
    bool m_value_isValid;
};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::Currency)

#endif // Currency_H