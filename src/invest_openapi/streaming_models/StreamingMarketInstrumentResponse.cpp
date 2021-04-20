/**
 */

#include "StreamingMarketInstrumentResponse.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>

#include "../models/client/Helpers.h"

namespace OpenAPI {

StreamingMarketInstrumentResponse::StreamingMarketInstrumentResponse(QString json) {
    this->initializeModel();
    this->fromJson(json);
}

StreamingMarketInstrumentResponse::StreamingMarketInstrumentResponse() {
    this->initializeModel();
}

StreamingMarketInstrumentResponse::~StreamingMarketInstrumentResponse() {}

void StreamingMarketInstrumentResponse::initializeModel() {

    m_event_isSet = false;
    m_event_isValid = false;

    m_time_isSet = false;
    m_time_isValid = false;

    m_payload_isSet = false;
    m_payload_isValid = false;
}

void StreamingMarketInstrumentResponse::fromJson(QString jsonString) {
    QByteArray array(jsonString.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = doc.object();
    this->fromJsonObject(jsonObject);
}

void StreamingMarketInstrumentResponse::fromJsonObject(QJsonObject json) {

    m_event_isValid = ::OpenAPI::fromJsonValue(event, json[QString("event")]);
    m_event_isSet = !json[QString("event")].isNull() && m_event_isValid;

    m_time_isValid = ::OpenAPI::fromJsonValue(time, json[QString("time")]);
    m_time_isSet = !json[QString("time")].isNull() && m_time_isValid;

    m_payload_isValid = ::OpenAPI::fromJsonValue(payload, json[QString("payload")]);
    m_payload_isSet = !json[QString("payload")].isNull() && m_payload_isValid;
}

QString StreamingMarketInstrumentResponse::asJson() const {
    QJsonObject obj = this->asJsonObject();
    QJsonDocument doc(obj);
    QByteArray bytes = doc.toJson();
    return QString(bytes);
}

QJsonObject StreamingMarketInstrumentResponse::asJsonObject() const {
    QJsonObject obj;
    if (m_event_isSet) {
        obj.insert(QString("event"), ::OpenAPI::toJsonValue(event));
    }
    if (m_time_isSet) {
        obj.insert(QString("time"), ::OpenAPI::toJsonValue(time));
    }
    if (payload.isSet()) {
        obj.insert(QString("payload"), ::OpenAPI::toJsonValue(payload));
    }
    return obj;
}

QString StreamingMarketInstrumentResponse::getEvent() const {
    return event;
}
void StreamingMarketInstrumentResponse::setEvent(const QString &event) {
    this->event = event;
    this->m_event_isSet = true;
}

bool StreamingMarketInstrumentResponse::is_event_Set() const{
    return m_event_isSet;
}

bool StreamingMarketInstrumentResponse::is_event_Valid() const{
    return m_event_isValid;
}


QDateTime StreamingMarketInstrumentResponse::getTime() const {
    return time;
}
void StreamingMarketInstrumentResponse::setTime(const QDateTime &time) {
    this->time = time;
    this->m_time_isSet = true;
}

bool StreamingMarketInstrumentResponse::is_time_Set() const{
    return m_time_isSet;
}

bool StreamingMarketInstrumentResponse::is_time_Valid() const{
    return m_time_isValid;
}

StreamingMarketInstrument StreamingMarketInstrumentResponse::getPayload() const {
    return payload;
}
void StreamingMarketInstrumentResponse::setPayload(const StreamingMarketInstrument &payload) {
    this->payload = payload;
    this->m_payload_isSet = true;
}

bool StreamingMarketInstrumentResponse::is_payload_Set() const{
    return m_payload_isSet;
}

bool StreamingMarketInstrumentResponse::is_payload_Valid() const{
    return m_payload_isValid;
}

bool StreamingMarketInstrumentResponse::isSet() const {
    bool isObjectUpdated = false;
    do {
        if (m_event_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_time_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (payload.isSet()) {
            isObjectUpdated = true;
            break;
        }
    } while (false);
    return isObjectUpdated;
}

bool StreamingMarketInstrumentResponse::isValid() const {
    // only required properties are required for the object to be considered valid
    return m_event_isValid && m_time_isValid && m_payload_isValid && true;
}

} // namespace OpenAPI
