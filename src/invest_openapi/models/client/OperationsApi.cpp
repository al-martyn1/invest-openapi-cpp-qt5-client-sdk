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

#include "OperationsApi.h"
#include "Helpers.h"

#include <QJsonArray>
#include <QJsonDocument>

namespace OpenAPI {

OperationsApi::OperationsApi(const QString &scheme, const QString &host, int port, const QString &basePath, const int timeOut)
    : _scheme(scheme),
      _host(host),
      _port(port),
      _basePath(basePath),
      _timeOut(timeOut),
      isResponseCompressionEnabled(false),
      isRequestCompressionEnabled(false) {}

OperationsApi::~OperationsApi() {
}

void OperationsApi::setScheme(const QString &scheme) {
    _scheme = scheme;
}

void OperationsApi::setHost(const QString &host) {
    _host = host;
}

void OperationsApi::setPort(int port) {
    _port = port;
}

void OperationsApi::setBasePath(const QString &basePath) {
    _basePath = basePath;
}

void OperationsApi::setTimeOut(const int timeOut) {
    _timeOut = timeOut;
}

void OperationsApi::setWorkingDirectory(const QString &path) {
    _workingDirectory = path;
}

void OperationsApi::addHeaders(const QString &key, const QString &value) {
    defaultHeaders.insert(key, value);
}

void OperationsApi::enableRequestCompression() {
    isRequestCompressionEnabled = true;
}

void OperationsApi::enableResponseCompression() {
    isResponseCompressionEnabled = true;
}

void OperationsApi::abortRequests(){
    emit abortRequestsSignal();
}

void OperationsApi::operationsGet(const QDateTime &from, const QDateTime &to, const QString &figi, const QString &broker_account_id) {
    QString fullPath = QString("%1://%2%3%4%5")
                           .arg(_scheme)
                           .arg(_host)
                           .arg(_port ? ":" + QString::number(_port) : "")
                           .arg(_basePath)
                           .arg("/operations");

    if (fullPath.indexOf("?") > 0)
        fullPath.append("&");
    else
        fullPath.append("?");
    fullPath.append(QUrl::toPercentEncoding("from")).append("=").append(QUrl::toPercentEncoding(::OpenAPI::toStringValue(from)));

    if (fullPath.indexOf("?") > 0)
        fullPath.append("&");
    else
        fullPath.append("?");
    fullPath.append(QUrl::toPercentEncoding("to")).append("=").append(QUrl::toPercentEncoding(::OpenAPI::toStringValue(to)));

    if (fullPath.indexOf("?") > 0)
        fullPath.append("&");
    else
        fullPath.append("?");
    fullPath.append(QUrl::toPercentEncoding("figi")).append("=").append(QUrl::toPercentEncoding(::OpenAPI::toStringValue(figi)));

    if (fullPath.indexOf("?") > 0)
        fullPath.append("&");
    else
        fullPath.append("?");
    fullPath.append(QUrl::toPercentEncoding("brokerAccountId")).append("=").append(QUrl::toPercentEncoding(::OpenAPI::toStringValue(broker_account_id)));

    HttpRequestWorker *worker = new HttpRequestWorker(this);
    worker->setTimeOut(_timeOut);
    worker->setWorkingDirectory(_workingDirectory);
    HttpRequestInput input(fullPath, "GET");

    foreach (QString key, this->defaultHeaders.keys()) { input.headers.insert(key, this->defaultHeaders.value(key)); }

    connect(worker, &HttpRequestWorker::on_execution_finished, this, &OperationsApi::operationsGetCallback);
    connect(this, &OperationsApi::abortRequestsSignal, worker, &QObject::deleteLater); 
    worker->execute(&input);
}

void OperationsApi::operationsGetCallback(HttpRequestWorker *worker) {
    QString msg;
    QString error_str = worker->error_str;
    QNetworkReply::NetworkError error_type = worker->error_type;

    if (worker->error_type == QNetworkReply::NoError) {
        msg = QString("Success! %1 bytes").arg(worker->response.length());
    } else {
        msg = "Error: " + worker->error_str;
        error_str = QString("%1, %2").arg(worker->error_str).arg(QString(worker->response));
    }
    OperationsResponse output(QString(worker->response));
    worker->deleteLater();

    if (worker->error_type == QNetworkReply::NoError) {
        emit operationsGetSignal(output);
        emit operationsGetSignalFull(worker, output);
    } else {
        emit operationsGetSignalE(output, error_type, error_str);
        emit operationsGetSignalEFull(worker, error_type, error_str);
    }
}

} // namespace OpenAPI