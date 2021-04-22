/**
 */

#include "StreamingInstrumentInfoResponse.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>

#include "../models/client/Helpers.h"

namespace OpenAPI {

StreamingInstrumentInfoResponse::StreamingInstrumentInfoResponse(QString json) {
    this->initializeModel();
    this->fromJson(json);
}

StreamingInstrumentInfoResponse::StreamingInstrumentInfoResponse() {
    this->initializeModel();
}

StreamingInstrumentInfoResponse::~StreamingInstrumentInfoResponse() {}

void StreamingInstrumentInfoResponse::initializeModel() {

    m_event_isSet = false;
    m_event_isValid = false;

    m_time_isSet = false;
    m_time_isValid = false;

    m_payload_isSet = false;
    m_payload_isValid = false;
}

void StreamingInstrumentInfoResponse::fromJson(QString jsonString) {
    QByteArray array(jsonString.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = doc.object();
    this->fromJsonObject(jsonObject);
}

void StreamingInstrumentInfoResponse::fromJsonObject(QJsonObject json) {

    m_event_isValid = ::OpenAPI::fromJsonValue(event, json[QString("event")]);
    m_event_isSet = !json[QString("event")].isNull() && m_event_isValid;

    m_time_isValid = ::OpenAPI::fromJsonValue(time, json[QString("time")]);
    m_time_isSet = !json[QString("time")].isNull() && m_time_isValid;

    m_payload_isValid = ::OpenAPI::fromJsonValue(payload, json[QString("payload")]);
    m_payload_isSet = !json[QString("payload")].isNull() && m_payload_isValid;
}

QString StreamingInstrumentInfoResponse::asJson() const {
    QJsonObject obj = this->asJsonObject();
    QJsonDocument doc(obj);
    QByteArray bytes = doc.toJson();
    return QString(bytes);
}

QJsonObject StreamingInstrumentInfoResponse::asJsonObject() const {
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

QString StreamingInstrumentInfoResponse::getEvent() const {
    return event;
}
void StreamingInstrumentInfoResponse::setEvent(const QString &event) {
    this->event = event;
    this->m_event_isSet = true;
}

bool StreamingInstrumentInfoResponse::is_event_Set() const{
    return m_event_isSet;
}

bool StreamingInstrumentInfoResponse::is_event_Valid() const{
    return m_event_isValid;
}


QDateTime StreamingInstrumentInfoResponse::getTime() const {
    return time;
}
void StreamingInstrumentInfoResponse::setTime(const QDateTime &time) {
    this->time = time;
    this->m_time_isSet = true;
}

bool StreamingInstrumentInfoResponse::is_time_Set() const{
    return m_time_isSet;
}

bool StreamingInstrumentInfoResponse::is_time_Valid() const{
    return m_time_isValid;
}

StreamingInstrumentInfo StreamingInstrumentInfoResponse::getPayload() const {
    return payload;
}
void StreamingInstrumentInfoResponse::setPayload(const StreamingInstrumentInfo &payload) {
    this->payload = payload;
    this->m_payload_isSet = true;
}

bool StreamingInstrumentInfoResponse::is_payload_Set() const{
    return m_payload_isSet;
}

bool StreamingInstrumentInfoResponse::is_payload_Valid() const{
    return m_payload_isValid;
}

bool StreamingInstrumentInfoResponse::isSet() const {
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

bool StreamingInstrumentInfoResponse::isValid() const {
    // only required properties are required for the object to be considered valid
    return m_event_isValid && m_time_isValid && m_payload_isValid && true;
}

} // namespace OpenAPI
