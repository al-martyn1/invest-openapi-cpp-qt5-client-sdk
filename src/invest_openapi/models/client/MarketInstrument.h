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
 * MarketInstrument.h
 *
 * 
 */

#ifndef MarketInstrument_H
#define MarketInstrument_H

#include <QJsonObject>

#include "Currency.h"
#include "InstrumentType.h"
#include <QString>

#include "Enum.h"
#include "Object.h"

#include "../../marty_decimal.h"


namespace OpenAPI {

class MarketInstrument : public Object {
public:
    MarketInstrument();
    MarketInstrument(QString json);
    ~MarketInstrument() override;

    QString asJson() const override;
    QJsonObject asJsonObject() const override;
    void fromJsonObject(QJsonObject json) override;
    void fromJson(QString jsonString) override;

    QString getFigi() const;
    void setFigi(const QString &figi);
    bool is_figi_Set() const;
    bool is_figi_Valid() const;

    QString getTicker() const;
    void setTicker(const QString &ticker);
    bool is_ticker_Set() const;
    bool is_ticker_Valid() const;

    QString getIsin() const;
    void setIsin(const QString &isin);
    bool is_isin_Set() const;
    bool is_isin_Valid() const;

    marty::Decimal getMinPriceIncrement() const;
    void setMinPriceIncrement(const marty::Decimal &min_price_increment);
    bool is_min_price_increment_Set() const;
    bool is_min_price_increment_Valid() const;

    qint32 getLot() const;
    void setLot(const qint32 &lot);
    bool is_lot_Set() const;
    bool is_lot_Valid() const;

    qint32 getMinQuantity() const;
    void setMinQuantity(const qint32 &min_quantity);
    bool is_min_quantity_Set() const;
    bool is_min_quantity_Valid() const;

    Currency getCurrency() const;
    void setCurrency(const Currency &currency);
    bool is_currency_Set() const;
    bool is_currency_Valid() const;

    QString getName() const;
    void setName(const QString &name);
    bool is_name_Set() const;
    bool is_name_Valid() const;

    InstrumentType getType() const;
    void setType(const InstrumentType &type);
    bool is_type_Set() const;
    bool is_type_Valid() const;

    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    QString figi;
    bool m_figi_isSet;
    bool m_figi_isValid;

    QString ticker;
    bool m_ticker_isSet;
    bool m_ticker_isValid;

    QString isin;
    bool m_isin_isSet;
    bool m_isin_isValid;

    marty::Decimal min_price_increment;
    bool m_min_price_increment_isSet;
    bool m_min_price_increment_isValid;

    qint32 lot;
    bool m_lot_isSet;
    bool m_lot_isValid;

    qint32 min_quantity;
    bool m_min_quantity_isSet;
    bool m_min_quantity_isValid;

    Currency currency;
    bool m_currency_isSet;
    bool m_currency_isValid;

    QString name;
    bool m_name_isSet;
    bool m_name_isValid;

    InstrumentType type;
    bool m_type_isSet;
    bool m_type_isValid;
};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::MarketInstrument)

#endif // MarketInstrument_H
