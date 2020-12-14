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

#ifndef _UserApi_H
#define _UserApi_H

#include "HttpRequest.h"

#include "Error.h"
#include "UserAccountsResponse.h"
#include <QString>

#include <QObject>

namespace OpenAPI {

class UserApi : public QObject {
    Q_OBJECT

public:
    UserApi(const QString &scheme = "https", const QString &host = "api-invest.tinkoff.ru", int port = 0, const QString &basePath = "/openapi", const int timeOut = 0);
    ~UserApi();

    void setScheme(const QString &scheme);
    void setHost(const QString &host);
    void setPort(int port);
    void setBasePath(const QString &basePath);
    void setTimeOut(const int timeOut);
    void setWorkingDirectory(const QString &path);
    void addHeaders(const QString &key, const QString &value);
    void enableRequestCompression();
    void enableResponseCompression();
    void abortRequests();

    void userAccountsGet();

private:
    QString _scheme, _host;
    int _port;
    QString _basePath;
    int _timeOut;
    QString _workingDirectory;
    QMap<QString, QString> defaultHeaders;
    bool isResponseCompressionEnabled;
    bool isRequestCompressionEnabled;

    void userAccountsGetCallback(HttpRequestWorker *worker);

signals:

    void userAccountsGetSignal(UserAccountsResponse summary);

    void userAccountsGetSignalFull(HttpRequestWorker *worker, UserAccountsResponse summary);

    void userAccountsGetSignalE(UserAccountsResponse summary, QNetworkReply::NetworkError error_type, QString error_str);

    void userAccountsGetSignalEFull(HttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str);

    void abortRequestsSignal(); 
};

} // namespace OpenAPI
#endif