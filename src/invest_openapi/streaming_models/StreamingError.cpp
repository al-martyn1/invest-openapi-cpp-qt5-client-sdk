/**
 */

#include "StreamingError.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>

#include "../models/client/Helpers.h"

namespace OpenAPI {

StreamingError::StreamingError(QString json) {
    this->initializeModel();
    this->fromJson(json);
}

StreamingError::StreamingError() {
    this->initializeModel();
}

StreamingError::~StreamingError() {}

void StreamingError::initializeModel() {

    m_event_isSet = false;
    m_event_isValid = false;

    m_time_isSet = false;
    m_time_isValid = false;

    m_payload_isSet = false;
    m_payload_isValid = false;
}

void StreamingError::fromJson(QString jsonString) {
    QByteArray array(jsonString.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = doc.object();
    this->fromJsonObject(jsonObject);
}

void StreamingError::fromJsonObject(QJsonObject json) {

    m_event_isValid = ::OpenAPI::fromJsonValue(event, json[QString("event")]);
    m_event_isSet = !json[QString("event")].isNull() && m_event_isValid;

    m_time_isValid = ::OpenAPI::fromJsonValue(time, json[QString("time")]);
    m_time_isSet = !json[QString("time")].isNull() && m_time_isValid;

    m_payload_isValid = ::OpenAPI::fromJsonValue(payload, json[QString("payload")]);
    m_payload_isSet = !json[QString("payload")].isNull() && m_payload_isValid;
}

QString StreamingError::asJson() const {
    QJsonObject obj = this->asJsonObject();
    QJsonDocument doc(obj);
    QByteArray bytes = doc.toJson();
    return QString(bytes);
}

QJsonObject StreamingError::asJsonObject() const {
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

QString StreamingError::getEvent() const {
    return event;
}
void StreamingError::setEvent(const QString &event) {
    this->event = event;
    this->m_event_isSet = true;
}

bool StreamingError::is_event_Set() const{
    return m_event_isSet;
}

bool StreamingError::is_event_Valid() const{
    return m_event_isValid;
}


QDateTime StreamingError::getTime() const {
    return time;
}
void StreamingError::setTime(const QDateTime &time) {
    this->time = time;
    this->m_time_isSet = true;
}

bool StreamingError::is_time_Set() const{
    return m_time_isSet;
}

bool StreamingError::is_time_Valid() const{
    return m_time_isValid;
}

StreamingError_payload StreamingError::getPayload() const {
    return payload;
}
void StreamingError::setPayload(const StreamingError_payload &payload) {
    this->payload = payload;
    this->m_payload_isSet = true;
}

bool StreamingError::is_payload_Set() const{
    return m_payload_isSet;
}

bool StreamingError::is_payload_Valid() const{
    return m_payload_isValid;
}

bool StreamingError::isSet() const {
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

bool StreamingError::isValid() const {
    // only required properties are required for the object to be considered valid
    return m_event_isValid && m_time_isValid && m_payload_isValid && true;
}

} // namespace OpenAPI
