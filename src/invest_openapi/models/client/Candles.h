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
 * Candles.h
 *
 * 
 */

#ifndef Candles_H
#define Candles_H

#include <QJsonObject>

#include "Candle.h"
#include "CandleResolution.h"
#include <QList>
#include <QString>

#include "Enum.h"
#include "Object.h"

namespace OpenAPI {

class Candles : public Object {
public:
    Candles();
    Candles(QString json);
    ~Candles() override;

    QString asJson() const override;
    QJsonObject asJsonObject() const override;
    void fromJsonObject(QJsonObject json) override;
    void fromJson(QString jsonString) override;

    QString getFigi() const;
    void setFigi(const QString &figi);
    bool is_figi_Set() const;
    bool is_figi_Valid() const;

    CandleResolution getInterval() const;
    void setInterval(const CandleResolution &interval);
    bool is_interval_Set() const;
    bool is_interval_Valid() const;

    QList<Candle> getCandles() const;
    void setCandles(const QList<Candle> &candles);
    bool is_candles_Set() const;
    bool is_candles_Valid() const;

    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    QString figi;
    bool m_figi_isSet;
    bool m_figi_isValid;

    CandleResolution interval;
    bool m_interval_isSet;
    bool m_interval_isValid;

    QList<Candle> candles;
    bool m_candles_isSet;
    bool m_candles_isValid;
};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::Candles)

#endif // Candles_H