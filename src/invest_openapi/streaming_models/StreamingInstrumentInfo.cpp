/**
 */

#include "StreamingInstrumentInfo.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>

#include "../models/client/Helpers.h"

namespace OpenAPI {

StreamingInstrumentInfo::StreamingInstrumentInfo(QString json) {
    this->initializeModel();
    this->fromJson(json);
}

StreamingInstrumentInfo::StreamingInstrumentInfo() {
    this->initializeModel();
}

StreamingInstrumentInfo::~StreamingInstrumentInfo() {}

void StreamingInstrumentInfo::initializeModel() {

    m_figi_isSet = false;
    m_figi_isValid = false;

    m_trade_status_isSet = false;
    m_trade_status_isValid = false;

    m_min_price_increment_isSet = false;
    m_min_price_increment_isValid = false;

    m_lot_isSet = false;
    m_lot_isValid = false;
}

void StreamingInstrumentInfo::fromJson(QString jsonString) {
    QByteArray array(jsonString.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = doc.object();
    this->fromJsonObject(jsonObject);
}

void StreamingInstrumentInfo::fromJsonObject(QJsonObject json) {

    m_figi_isValid = ::OpenAPI::fromJsonValue(figi, json[QString("figi")]);
    m_figi_isSet = !json[QString("figi")].isNull() && m_figi_isValid;

    m_trade_status_isValid = ::OpenAPI::fromJsonValue(trade_status, json[QString("trade_status")]);
    m_trade_status_isSet = !json[QString("trade_status")].isNull() && m_trade_status_isValid;

    m_min_price_increment_isValid = ::OpenAPI::fromJsonValue(min_price_increment, json[QString("min_price_increment")]);
    m_min_price_increment_isSet = !json[QString("min_price_increment")].isNull() && m_min_price_increment_isValid;

    m_lot_isValid = ::OpenAPI::fromJsonValue(lot, json[QString("lot")]);
    m_lot_isSet = !json[QString("lot")].isNull() && m_lot_isValid;
}

QString StreamingInstrumentInfo::asJson() const {
    QJsonObject obj = this->asJsonObject();
    QJsonDocument doc(obj);
    QByteArray bytes = doc.toJson();
    return QString(bytes);
}

QJsonObject StreamingInstrumentInfo::asJsonObject() const {
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

QString StreamingInstrumentInfo::getFigi() const {
    return figi;
}
void StreamingInstrumentInfo::setFigi(const QString &figi) {
    this->figi = figi;
    this->m_figi_isSet = true;
}

bool StreamingInstrumentInfo::is_figi_Set() const{
    return m_figi_isSet;
}

bool StreamingInstrumentInfo::is_figi_Valid() const{
    return m_figi_isValid;
}

TradeStatus StreamingInstrumentInfo::getTradeStatus() const {
    return trade_status;
}
void StreamingInstrumentInfo::setTradeStatus(const TradeStatus &trade_status) {
    this->trade_status = trade_status;
    this->m_trade_status_isSet = true;
}

bool StreamingInstrumentInfo::is_trade_status_Set() const{
    return m_trade_status_isSet;
}

bool StreamingInstrumentInfo::is_trade_status_Valid() const{
    return m_trade_status_isValid;
}

marty::Decimal StreamingInstrumentInfo::getMinPriceIncrement() const {
    return min_price_increment;
}
void StreamingInstrumentInfo::setMinPriceIncrement(const marty::Decimal &min_price_increment) {
    this->min_price_increment = min_price_increment;
    this->m_min_price_increment_isSet = true;
}

bool StreamingInstrumentInfo::is_min_price_increment_Set() const{
    return m_min_price_increment_isSet;
}

bool StreamingInstrumentInfo::is_min_price_increment_Valid() const{
    return m_min_price_increment_isValid;
}

qint32 StreamingInstrumentInfo::getLot() const {
    return lot;
}
void StreamingInstrumentInfo::setLot(const qint32 &lot) {
    this->lot = lot;
    this->m_lot_isSet = true;
}

bool StreamingInstrumentInfo::is_lot_Set() const{
    return m_lot_isSet;
}

bool StreamingInstrumentInfo::is_lot_Valid() const{
    return m_lot_isValid;
}

bool StreamingInstrumentInfo::isSet() const {
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

bool StreamingInstrumentInfo::isValid() const {
    // only required properties are required for the object to be considered valid
    return m_figi_isValid && m_trade_status_isValid && m_min_price_increment_isValid && m_lot_isValid && true;
}

} // namespace OpenAPI
