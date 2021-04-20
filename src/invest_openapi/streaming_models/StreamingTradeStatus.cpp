/**
 */

#include "StreamingTradeStatus.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>

#include "../models/client/Helpers.h"

namespace OpenAPI {

StreamingTradeStatus::StreamingTradeStatus(QString json) {
    this->initializeModel();
    this->fromJson(json);
}

StreamingTradeStatus::StreamingTradeStatus() {
    this->initializeModel();
}

StreamingTradeStatus::~StreamingTradeStatus() {}

void StreamingTradeStatus::initializeModel() {

    m_value_isSet = false;
    m_value_isValid = false;
    m_value = eStreamingTradeStatus::INVALID_VALUE_OPENAPI_GENERATED;
}

void StreamingTradeStatus::fromJson(QString jsonString) {
    
    if ( jsonString.compare("normal_trading", Qt::CaseInsensitive) == 0) {
        m_value = eStreamingTradeStatus::NORMALTRADING;
        m_value_isSet = m_value_isValid = true;
    }
    else // if ( jsonString.compare("NotAvailableForTrading", Qt::CaseInsensitive) == 0) {
    {
        m_value = eStreamingTradeStatus::NOTAVAILABLEFORTRADING;
        m_value_isSet = m_value_isValid = true;
    }
}

void StreamingTradeStatus::fromJsonValue(QJsonValue json) {
fromJson(json.toString());
}

QString StreamingTradeStatus::asJson() const {
    
    QString val;
    switch (m_value){
        case eStreamingTradeStatus::NORMALTRADING:
            val = "normal_trading";
            break;
        case eStreamingTradeStatus::NOTAVAILABLEFORTRADING:
            val = "NotAvailableForTrading";
            break;
        default:
            break;
    }
    return val;
}

QJsonValue StreamingTradeStatus::asJsonValue() const {
    
    return QJsonValue(asJson());
}


StreamingTradeStatus::eStreamingTradeStatus StreamingTradeStatus::getValue() const {
    return m_value;
}

void StreamingTradeStatus::setValue(const StreamingTradeStatus::eStreamingTradeStatus& value){
    m_value = value;
    m_value_isSet = true;
}
bool StreamingTradeStatus::isSet() const {
    
    return m_value_isSet;
}

bool StreamingTradeStatus::isValid() const {
    // only required properties are required for the object to be considered valid
    return m_value_isValid;
}

} // namespace OpenAPI
