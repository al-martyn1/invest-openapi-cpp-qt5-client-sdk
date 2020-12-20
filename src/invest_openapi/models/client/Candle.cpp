/**
 * OpenAPI
 * tinkoff.ru/invest OpenAPI.
 *
 * The version of the OpenAPI document: 1.0.0
 * Contact: n.v.melnikov@tinkoff.ru
 *
 * NOTE: This class is auto generated by OpenAPI Generator (https://openapi-generator.tech).
 * https://openapi-generator.tech
 * Do not edit the class manually.
 */

#include "Candle.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>

#include "Helpers.h"

namespace OpenAPI {

Candle::Candle(QString json) {
    this->initializeModel();
    this->fromJson(json);
}

Candle::Candle() {
    this->initializeModel();
}

Candle::~Candle() {}

void Candle::initializeModel() {

    m_figi_isSet = false;
    m_figi_isValid = false;

    m_interval_isSet = false;
    m_interval_isValid = false;

    m_o_isSet = false;
    m_o_isValid = false;

    m_c_isSet = false;
    m_c_isValid = false;

    m_h_isSet = false;
    m_h_isValid = false;

    m_l_isSet = false;
    m_l_isValid = false;

    m_v_isSet = false;
    m_v_isValid = false;

    m_time_isSet = false;
    m_time_isValid = false;
}

void Candle::fromJson(QString jsonString) {
    QByteArray array(jsonString.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = doc.object();
    this->fromJsonObject(jsonObject);
}

void Candle::fromJsonObject(QJsonObject json) {

    m_figi_isValid = ::OpenAPI::fromJsonValue(figi, json[QString("figi")]);
    m_figi_isSet = !json[QString("figi")].isNull() && m_figi_isValid;

    m_interval_isValid = ::OpenAPI::fromJsonValue(interval, json[QString("interval")]);
    m_interval_isSet = !json[QString("interval")].isNull() && m_interval_isValid;

    m_o_isValid = ::OpenAPI::fromJsonValue(o, json[QString("o")]);
    m_o_isSet = !json[QString("o")].isNull() && m_o_isValid;

    m_c_isValid = ::OpenAPI::fromJsonValue(c, json[QString("c")]);
    m_c_isSet = !json[QString("c")].isNull() && m_c_isValid;

    m_h_isValid = ::OpenAPI::fromJsonValue(h, json[QString("h")]);
    m_h_isSet = !json[QString("h")].isNull() && m_h_isValid;

    m_l_isValid = ::OpenAPI::fromJsonValue(l, json[QString("l")]);
    m_l_isSet = !json[QString("l")].isNull() && m_l_isValid;

    m_v_isValid = ::OpenAPI::fromJsonValue(v, json[QString("v")]);
    m_v_isSet = !json[QString("v")].isNull() && m_v_isValid;

    m_time_isValid = ::OpenAPI::fromJsonValue(time, json[QString("time")]);
    m_time_isSet = !json[QString("time")].isNull() && m_time_isValid;
}

QString Candle::asJson() const {
    QJsonObject obj = this->asJsonObject();
    QJsonDocument doc(obj);
    QByteArray bytes = doc.toJson();
    return QString(bytes);
}

QJsonObject Candle::asJsonObject() const {
    QJsonObject obj;
    if (m_figi_isSet) {
        obj.insert(QString("figi"), ::OpenAPI::toJsonValue(figi));
    }
    if (interval.isSet()) {
        obj.insert(QString("interval"), ::OpenAPI::toJsonValue(interval));
    }
    if (m_o_isSet) {
        obj.insert(QString("o"), ::OpenAPI::toJsonValue(o));
    }
    if (m_c_isSet) {
        obj.insert(QString("c"), ::OpenAPI::toJsonValue(c));
    }
    if (m_h_isSet) {
        obj.insert(QString("h"), ::OpenAPI::toJsonValue(h));
    }
    if (m_l_isSet) {
        obj.insert(QString("l"), ::OpenAPI::toJsonValue(l));
    }
    if (m_v_isSet) {
        obj.insert(QString("v"), ::OpenAPI::toJsonValue(v));
    }
    if (m_time_isSet) {
        obj.insert(QString("time"), ::OpenAPI::toJsonValue(time));
    }
    return obj;
}

QString Candle::getFigi() const {
    return figi;
}
void Candle::setFigi(const QString &figi) {
    this->figi = figi;
    this->m_figi_isSet = true;
}

bool Candle::is_figi_Set() const{
    return m_figi_isSet;
}

bool Candle::is_figi_Valid() const{
    return m_figi_isValid;
}

CandleResolution Candle::getInterval() const {
    return interval;
}
void Candle::setInterval(const CandleResolution &interval) {
    this->interval = interval;
    this->m_interval_isSet = true;
}

bool Candle::is_interval_Set() const{
    return m_interval_isSet;
}

bool Candle::is_interval_Valid() const{
    return m_interval_isValid;
}

double Candle::getO() const {
    return o;
}
void Candle::setO(const double &o) {
    this->o = o;
    this->m_o_isSet = true;
}

bool Candle::is_o_Set() const{
    return m_o_isSet;
}

bool Candle::is_o_Valid() const{
    return m_o_isValid;
}

double Candle::getC() const {
    return c;
}
void Candle::setC(const double &c) {
    this->c = c;
    this->m_c_isSet = true;
}

bool Candle::is_c_Set() const{
    return m_c_isSet;
}

bool Candle::is_c_Valid() const{
    return m_c_isValid;
}

double Candle::getH() const {
    return h;
}
void Candle::setH(const double &h) {
    this->h = h;
    this->m_h_isSet = true;
}

bool Candle::is_h_Set() const{
    return m_h_isSet;
}

bool Candle::is_h_Valid() const{
    return m_h_isValid;
}

double Candle::getL() const {
    return l;
}
void Candle::setL(const double &l) {
    this->l = l;
    this->m_l_isSet = true;
}

bool Candle::is_l_Set() const{
    return m_l_isSet;
}

bool Candle::is_l_Valid() const{
    return m_l_isValid;
}

qint32 Candle::getV() const {
    return v;
}
void Candle::setV(const qint32 &v) {
    this->v = v;
    this->m_v_isSet = true;
}

bool Candle::is_v_Set() const{
    return m_v_isSet;
}

bool Candle::is_v_Valid() const{
    return m_v_isValid;
}

QDateTime Candle::getTime() const {
    return time;
}
void Candle::setTime(const QDateTime &time) {
    this->time = time;
    this->m_time_isSet = true;
}

bool Candle::is_time_Set() const{
    return m_time_isSet;
}

bool Candle::is_time_Valid() const{
    return m_time_isValid;
}

bool Candle::isSet() const {
    bool isObjectUpdated = false;
    do {
        if (m_figi_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (interval.isSet()) {
            isObjectUpdated = true;
            break;
        }

        if (m_o_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_c_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_h_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_l_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_v_isSet) {
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

bool Candle::isValid() const {
    // only required properties are required for the object to be considered valid
    return m_figi_isValid && m_interval_isValid && m_o_isValid && m_c_isValid && m_h_isValid && m_l_isValid && m_v_isValid && m_time_isValid && true;
}

} // namespace OpenAPI