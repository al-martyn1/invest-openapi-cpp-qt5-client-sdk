/**
 */

#include "StreamingMarketInstrument.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>

#include "../models/client/Helpers.h"

namespace OpenAPI {

StreamingMarketInstrument::StreamingMarketInstrument(QString json) {
    this->initializeModel();
    this->fromJson(json);
}

StreamingMarketInstrument::StreamingMarketInstrument() {
    this->initializeModel();
}

StreamingMarketInstrument::~StreamingMarketInstrument() {}

void StreamingMarketInstrument::initializeModel() {

    m_figi_isSet = false;
    m_figi_isValid = false;

    m_trade_status_isSet = false;
    m_trade_status_isValid = false;

    m_min_price_increment_isSet = false;
    m_min_price_increment_isValid = false;

    m_lot_isSet = false;
    m_lot_isValid = false;
}

void StreamingMarketInstrument::fromJson(QString jsonString) {
    QByteArray array(jsonString.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = doc.object();
    this->fromJsonObject(jsonObject);
}

void StreamingMarketInstrument::fromJsonObject(QJsonObject json) {

    m_figi_isValid = ::OpenAPI::fromJsonValue(figi, json[QString("figi")]);
    m_figi_isSet = !json[QString("figi")].isNull() && m_figi_isValid;

    m_trade_status_isValid = ::OpenAPI::fromJsonValue(trade_status, json[QString("trade_status")]);
    m_trade_status_isSet = !json[QString("trade_status")].isNull() && m_trade_status_isValid;

    m_min_price_increment_isValid = ::OpenAPI::fromJsonValue(min_price_increment, json[QString("min_price_increment")]);
    m_min_price_increment_isSet = !json[QString("min_price_increment")].isNull() && m_min_price_increment_isValid;

    m_lot_isValid = ::OpenAPI::fromJsonValue(lot, json[QString("lot")]);
    m_lot_isSet = !json[QString("lot")].isNull() && m_lot_isValid;
}

QString StreamingMarketInstrument::asJson() const {
    QJsonObject obj = this->asJsonObject();
    QJsonDocument doc(obj);
    QByteArray bytes = doc.toJson();
    return QString(bytes);
}

QJsonObject StreamingMarketInstrument::asJsonObject() const {
    QJsonObject obj;
    if (m_figi_isSet) {
        obj.insert(QString("figi"), ::OpenAPI::toJsonValue(figi));
    }
    if (trade_status.isSet()) {
        obj.insert(QString("trade_status"), ::OpenAPI::toJsonValue(trade_status));
    }
    if (m_min_price_increment_isSet) {
        obj.insert(QString("min_price_increment"), ::OpenAPI::toJsonValue(min_price_increment));
    }
    if (m_lot_isSet) {
        obj.insert(QString("lot"), ::OpenAPI::toJsonValue(lot));
    }
    return obj;
}

QString StreamingMarketInstrument::getFigi() const {
    return figi;
}
void StreamingMarketInstrument::setFigi(const QString &figi) {
    this->figi = figi;
    this->m_figi_isSet = true;
}

bool StreamingMarketInstrument::is_figi_Set() const{
    return m_figi_isSet;
}

bool StreamingMarketInstrument::is_figi_Valid() const{
    return m_figi_isValid;
}

TradeStatus StreamingMarketInstrument::getTradeStatus() const {
    return trade_status;
}
void StreamingMarketInstrument::setTradeStatus(const TradeStatus &trade_status) {
    this->trade_status = trade_status;
    this->m_trade_status_isSet = true;
}

bool StreamingMarketInstrument::is_trade_status_Set() const{
    return m_trade_status_isSet;
}

bool StreamingMarketInstrument::is_trade_status_Valid() const{
    return m_trade_status_isValid;
}

marty::Decimal StreamingMarketInstrument::getMinPriceIncrement() const {
    return min_price_increment;
}
void StreamingMarketInstrument::setMinPriceIncrement(const marty::Decimal &min_price_increment) {
    this->min_price_increment = min_price_increment;
    this->m_min_price_increment_isSet = true;
}

bool StreamingMarketInstrument::is_min_price_increment_Set() const{
    return m_min_price_increment_isSet;
}

bool StreamingMarketInstrument::is_min_price_increment_Valid() const{
    return m_min_price_increment_isValid;
}

qint32 StreamingMarketInstrument::getLot() const {
    return lot;
}
void StreamingMarketInstrument::setLot(const qint32 &lot) {
    this->lot = lot;
    this->m_lot_isSet = true;
}

bool StreamingMarketInstrument::is_lot_Set() const{
    return m_lot_isSet;
}

bool StreamingMarketInstrument::is_lot_Valid() const{
    return m_lot_isValid;
}

bool StreamingMarketInstrument::isSet() const {
    bool isObjectUpdated = false;
    do {
        if (m_figi_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (trade_status.isSet()) {
            isObjectUpdated = true;
            break;
        }

        if (m_min_price_increment_isSet) {
            isObjectUpdated = true;
            break;
        }
    } while (false);
    return isObjectUpdated;
}

bool StreamingMarketInstrument::isValid() const {
    // only required properties are required for the object to be considered valid
    return m_figi_isValid && m_trade_status_isValid && m_min_price_increment_isValid && m_lot_isValid && true;
}

} // namespace OpenAPI
