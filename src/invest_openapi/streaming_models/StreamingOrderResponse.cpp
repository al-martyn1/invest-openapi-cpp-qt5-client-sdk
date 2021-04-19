/**
 */

#include "StreamingOrderResponse.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>

#include "../models/client/Helpers.h"

namespace OpenAPI {

StreamingOrderResponse::StreamingOrderResponse(QString json) {
    this->initializeModel();
    this->fromJson(json);
}

StreamingOrderResponse::StreamingOrderResponse() {
    this->initializeModel();
}

StreamingOrderResponse::~StreamingOrderResponse() {}

void StreamingOrderResponse::initializeModel() {

    m_price_isSet = false;
    m_price_isValid = false;

    m_quantity_isSet = false;
    m_quantity_isValid = false;
}

void StreamingOrderResponse::fromJson(QString jsonString) {
    QByteArray array(jsonString.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = doc.object();
    this->fromJsonObject(jsonObject);
}

void StreamingOrderResponse::fromJsonObject(QJsonObject json) {
/*
    m_price_isValid = ::OpenAPI::fromJsonValue(price, json[QString("price")]);
    m_price_isSet = !json[QString("price")].isNull() && m_price_isValid;

    m_quantity_isValid = ::OpenAPI::fromJsonValue(quantity, json[QString("quantity")]);
    m_quantity_isSet = !json[QString("quantity")].isNull() && m_quantity_isValid;
*/
}

QString StreamingOrderResponse::asJson() const {
    QJsonObject obj = this->asJsonObject();
    QJsonDocument doc(obj);
    QByteArray bytes = doc.toJson();
    return QString(bytes);
}

QJsonObject StreamingOrderResponse::asJsonObject() const {
    QJsonObject obj;
/*
    if (m_price_isSet) {
        obj.insert(QString("price"), ::OpenAPI::toJsonValue(price));
    }
    if (m_quantity_isSet) {
        obj.insert(QString("quantity"), ::OpenAPI::toJsonValue(quantity));
    }
*/
    return obj;
}

QList<marty::Decimal> StreamingOrderResponse::getValues() const {
    return values;
}
void StreamingOrderResponse::setValues(const QList<marty::Decimal> &values) {
    this->values = values;
    this->m_values_isSet = true;
}

bool StreamingOrderResponse::is_values_Set() const{
    return m_values_isSet;
}

bool StreamingOrderResponse::is_values_Valid() const{
    return m_values_isValid;
}

bool StreamingOrderResponse::isSet() const {
    bool isObjectUpdated = false;
    do {
        if (m_values_isSet) {
            isObjectUpdated = true;
            break;
        }

    } while (false);
    return isObjectUpdated;
}

bool StreamingOrderResponse::isValid() const {
    // only required properties are required for the object to be considered valid
    return m_values_isValid && true;
}

} // namespace OpenAPI
