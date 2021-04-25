/**
 */

#include "GenericStreamingResponse.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>

#include "../models/client/Helpers.h"

namespace OpenAPI {

GenericStreamingResponse::GenericStreamingResponse(QString json) {
    this->initializeModel();
    this->fromJson(json);
}

GenericStreamingResponse::GenericStreamingResponse() {
    this->initializeModel();
}

GenericStreamingResponse::~GenericStreamingResponse() {}

void GenericStreamingResponse::fromJson(QString jsonString) {
    QByteArray array(jsonString.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = doc.object();
    this->fromJsonObject(jsonObject);
}

QString GenericStreamingResponse::asJson() const {
    QJsonObject obj = this->asJsonObject();
    QJsonDocument doc(obj);
    QByteArray bytes = doc.toJson();
    return QString(bytes);
}

//----------------------------------------------------------------------------
void GenericStreamingResponse::initializeModel() {

    m_event_isSet = false;
    m_event_isValid = false;

    m_time_isSet = false;
    m_time_isValid = false;

}

void GenericStreamingResponse::fromJsonObject(QJsonObject json) {

    m_event_isValid = ::OpenAPI::fromJsonValue(event, json[QString("event")]);
    m_event_isSet = !json[QString("event")].isNull() && m_event_isValid;

    m_time_isValid = ::OpenAPI::fromJsonValue(time, json[QString("time")]);
    m_time_isSet = !json[QString("time")].isNull() && m_time_isValid;

    ::OpenAPI::fromJsonValue(timeString, json[QString("time")]);
}

QJsonObject GenericStreamingResponse::asJsonObject() const {
    QJsonObject obj;
    if (m_event_isSet) {
        obj.insert(QString("event"), ::OpenAPI::toJsonValue(event));
    }
    if (m_time_isSet) {
        obj.insert(QString("time"), ::OpenAPI::toJsonValue(time));
    }
    return obj;
}

QString GenericStreamingResponse::getEvent() const {
    return event;
}
void GenericStreamingResponse::setEvent(const QString &event) {
    this->event = event;
    this->m_event_isSet = true;
}

bool GenericStreamingResponse::is_event_Set() const{
    return m_event_isSet;
}

bool GenericStreamingResponse::is_event_Valid() const{
    return m_event_isValid;
}


QDateTime GenericStreamingResponse::getTime() const {
    return time;
}
void GenericStreamingResponse::setTime(const QDateTime &time) {
    this->time = time;
    this->m_time_isSet = true;
}

bool GenericStreamingResponse::is_time_Set() const{
    return m_time_isSet;
}

bool GenericStreamingResponse::is_time_Valid() const{
    return m_time_isValid;
}

QString GenericStreamingResponse::getTimeAsString() const
{
    return timeString;
}

bool GenericStreamingResponse::isSet() const {
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
    } while (false);
    return isObjectUpdated;
}

bool GenericStreamingResponse::isValid() const {
    // only required properties are required for the object to be considered valid
    return m_event_isValid && m_time_isValid && true;
}

//----------------------------------------------------------------------------

} // namespace OpenAPI
