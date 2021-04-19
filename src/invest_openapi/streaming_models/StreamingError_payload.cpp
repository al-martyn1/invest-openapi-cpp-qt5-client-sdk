/**
 */

#include "StreamingError_payload.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>

#include "../models/client/Helpers.h"

namespace OpenAPI {

StreamingError_payload::StreamingError_payload(QString json) {
    this->initializeModel();
    this->fromJson(json);
}

StreamingError_payload::StreamingError_payload() {
    this->initializeModel();
}

StreamingError_payload::~StreamingError_payload() {}

void StreamingError_payload::initializeModel() {

    m_message_isSet = false;
    m_message_isValid = false;
}

void StreamingError_payload::fromJson(QString jsonString) {
    QByteArray array(jsonString.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = doc.object();
    this->fromJsonObject(jsonObject);
}

void StreamingError_payload::fromJsonObject(QJsonObject json) {

    m_message_isValid = ::OpenAPI::fromJsonValue(message, json[QString("error")]);
    m_message_isSet = !json[QString("error")].isNull() && m_message_isValid;
}

QString StreamingError_payload::asJson() const {
    QJsonObject obj = this->asJsonObject();
    QJsonDocument doc(obj);
    QByteArray bytes = doc.toJson();
    return QString(bytes);
}

QJsonObject StreamingError_payload::asJsonObject() const {
    QJsonObject obj;
    if (m_message_isSet) {
        obj.insert(QString("error"), ::OpenAPI::toJsonValue(message));
    }
    return obj;
}

QString StreamingError_payload::getMessage() const {
    return message;
}
void StreamingError_payload::setMessage(const QString &message) {
    this->message = message;
    this->m_message_isSet = true;
}

bool StreamingError_payload::is_message_Set() const{
    return m_message_isSet;
}

bool StreamingError_payload::is_message_Valid() const{
    return m_message_isValid;
}

bool StreamingError_payload::isSet() const {
    bool isObjectUpdated = false;
    do {
        if (m_message_isSet) {
            isObjectUpdated = true;
            break;
        }

    } while (false);
    return isObjectUpdated;
}

bool StreamingError_payload::isValid() const {
    // only required properties are required for the object to be considered valid
    return true;
}

} // namespace OpenAPI
