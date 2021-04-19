/**
 */

#include "StreamingOrderbook.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>

#include "../models/client/Helpers.h"

namespace OpenAPI {

StreamingOrderbook::StreamingOrderbook(QString json) {
    this->initializeModel();
    this->fromJson(json);
}

StreamingOrderbook::StreamingOrderbook() {
    this->initializeModel();
}

StreamingOrderbook::~StreamingOrderbook() {}

void StreamingOrderbook::initializeModel() {

    m_figi_isSet = false;
    m_figi_isValid = false;

    m_depth_isSet = false;
    m_depth_isValid = false;

    m_bids_isSet = false;
    m_bids_isValid = false;

    m_asks_isSet = false;
    m_asks_isValid = false;
}

void StreamingOrderbook::fromJson(QString jsonString) {
    QByteArray array(jsonString.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = doc.object();
    this->fromJsonObject(jsonObject);
}

void StreamingOrderbook::fromJsonObject(QJsonObject json) {

    m_figi_isValid = ::OpenAPI::fromJsonValue(figi, json[QString("figi")]);
    m_figi_isSet = !json[QString("figi")].isNull() && m_figi_isValid;

    m_depth_isValid = ::OpenAPI::fromJsonValue(depth, json[QString("depth")]);
    m_depth_isSet = !json[QString("depth")].isNull() && m_depth_isValid;

    m_bids_isValid = ::OpenAPI::fromJsonValue(bids, json[QString("bids")]);
    m_bids_isSet = !json[QString("bids")].isNull() && m_bids_isValid;

    m_asks_isValid = ::OpenAPI::fromJsonValue(asks, json[QString("asks")]);
    m_asks_isSet = !json[QString("asks")].isNull() && m_asks_isValid;

}

QString StreamingOrderbook::asJson() const {
    QJsonObject obj = this->asJsonObject();
    QJsonDocument doc(obj);
    QByteArray bytes = doc.toJson();
    return QString(bytes);
}

QJsonObject StreamingOrderbook::asJsonObject() const {
    QJsonObject obj;
    if (m_figi_isSet) {
        obj.insert(QString("figi"), ::OpenAPI::toJsonValue(figi));
    }
    if (m_depth_isSet) {
        obj.insert(QString("depth"), ::OpenAPI::toJsonValue(depth));
    }
    if (bids.size() > 0) {
        obj.insert(QString("bids"), ::OpenAPI::toJsonValue(bids));
    }
    if (asks.size() > 0) {
        obj.insert(QString("asks"), ::OpenAPI::toJsonValue(asks));
    }
    return obj;
}

QString StreamingOrderbook::getFigi() const {
    return figi;
}
void StreamingOrderbook::setFigi(const QString &figi) {
    this->figi = figi;
    this->m_figi_isSet = true;
}

bool StreamingOrderbook::is_figi_Set() const{
    return m_figi_isSet;
}

bool StreamingOrderbook::is_figi_Valid() const{
    return m_figi_isValid;
}

qint32 StreamingOrderbook::getDepth() const {
    return depth;
}
void StreamingOrderbook::setDepth(const qint32 &depth) {
    this->depth = depth;
    this->m_depth_isSet = true;
}

bool StreamingOrderbook::is_depth_Set() const{
    return m_depth_isSet;
}

bool StreamingOrderbook::is_depth_Valid() const{
    return m_depth_isValid;
}

QList< QList<marty::Decimal> > StreamingOrderbook::getBids() const {
    return bids;
}
void StreamingOrderbook::setBids(const QList< QList<marty::Decimal> > &bids) {
    this->bids = bids;
    this->m_bids_isSet = true;
}

bool StreamingOrderbook::is_bids_Set() const{
    return m_bids_isSet;
}

bool StreamingOrderbook::is_bids_Valid() const{
    return m_bids_isValid;
}

QList< QList<marty::Decimal> > StreamingOrderbook::getAsks() const {
    return asks;
}
void StreamingOrderbook::setAsks(const QList< QList<marty::Decimal> > &asks) {
    this->asks = asks;
    this->m_asks_isSet = true;
}

bool StreamingOrderbook::is_asks_Set() const{
    return m_asks_isSet;
}

bool StreamingOrderbook::is_asks_Valid() const{
    return m_asks_isValid;
}

bool StreamingOrderbook::isSet() const {
    bool isObjectUpdated = false;
    do {
        if (m_figi_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_depth_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (bids.size() > 0) {
            isObjectUpdated = true;
            break;
        }

        if (asks.size() > 0) {
            isObjectUpdated = true;
            break;
        }

    } while (false);
    return isObjectUpdated;
}

bool StreamingOrderbook::isValid() const {
    // only required properties are required for the object to be considered valid
    return m_figi_isValid && m_depth_isValid && m_bids_isValid && m_asks_isValid && true;
}

} // namespace OpenAPI
