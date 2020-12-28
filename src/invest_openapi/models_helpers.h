/*! \file
    \brief 
 */

//----------------------------------------------------------------------------
#pragma once

#include <QObject>
#include <QDebug>
#include <QtDebug>
#include <QString>
#include <QDateTime>
#include <QNetworkRequest>
#include <QNetworkReply>

// D:\Qt\Qt5.14.0\5.14.0\msvc2017_64\include\QtNetwork\5.14.0\QtNetwork\private\ 
// qnetworkreplyhttpimpl_p.h

//#include "qdebug_support.h"

#include "models/client/HttpRequest.h"

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
namespace OpenAPI {



//----------------------------------------------------------------------------
void setRequestsDebug( bool v );
bool getRequestsDebug( );
void setResponsesDebug( bool v );
bool getResponsesDebug( );

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline
QString openapiHelpersFixGetUtcOffsetNumericStr( const QDateTime &dt )
{
    int utcOffset = dt.offsetFromUtc();

    QString utcStr;
    if (utcOffset<0)
    {
        utcStr.append("-");
        utcOffset = -utcOffset;
    }
    else
    {
        utcStr.append("+");
    }

    utcOffset /= 60;
    int hours   = utcOffset/60;
    int minutes = utcOffset%60;

    return utcStr + QString::asprintf("%02d:%02d", hours, minutes );
}

//----------------------------------------------------------------------------
inline
QString formatDateTimeISO8601( const QDateTime &value )
{
    return value.toString("yyyy-MM-ddThh:mm:ss.zzz")
    + QString("000")
    + openapiHelpersFixGetUtcOffsetNumericStr(value)
    ;
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
void dumpHttpRequestInput( const HttpRequestInput &input )
{
    if (!getRequestsDebug())
        return;

    // default mode is space() and quote()

    qDebug().nospace().noquote() << "----- REQUEST -----";

    qDebug().nospace().noquote() << input.http_method << " " << input.url_str;

    foreach (QString key, input.headers.keys())
    {
        qDebug().nospace().noquote() << key << ": " << input.headers.value(key);
    }

    //qDebug().nospace().noquote() << "\n";
    qDebug().nospace().noquote() << "--- BODY";

    qDebug().nospace().noquote() << input.request_body;

    qDebug().nospace().noquote() << "--- REQUEST END";
    qDebug().nospace().noquote() << Qt::endl;

}

//----------------------------------------------------------------------------
inline
void dumpHttpRequestInput( const HttpRequestInput &input, const QNetworkRequest &request )
{
    if (!getRequestsDebug())
        return;

    qDebug().nospace().noquote() << Qt::endl;
    qDebug().nospace().noquote() << "----- REQUEST -----";

    qDebug().nospace().noquote() << input.http_method << " " << input.url_str;
    //qDebug().nospace().noquote() << request.url().toString();

    auto headers = request.rawHeaderList();
    foreach (auto key, headers)
    {
        qDebug().nospace().noquote() << key << ": " << request.rawHeader(key);
    }

    //qDebug().nospace().noquote() << Qt::endl;
    qDebug().nospace().noquote() << "--- BODY";

    qDebug().nospace().noquote() << input.request_body;

    qDebug().nospace().noquote() << "--- REQUEST END";
    qDebug().nospace().noquote() << Qt::endl;

}

//----------------------------------------------------------------------------
#define INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( err ) case err : return #err

inline
QString toString( QNetworkReply::NetworkError e )
{
    switch(e)
    {
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::NoError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::ConnectionRefusedError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::RemoteHostClosedError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::HostNotFoundError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::TimeoutError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::OperationCanceledError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::SslHandshakeFailedError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::TemporaryNetworkFailureError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::NetworkSessionFailedError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::BackgroundRequestNotAllowedError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::TooManyRedirectsError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::InsecureRedirectError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::ProxyConnectionRefusedError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::ProxyConnectionClosedError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::ProxyNotFoundError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::ProxyTimeoutError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::ProxyAuthenticationRequiredError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::ContentAccessDenied );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::ContentOperationNotPermittedError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::ContentNotFoundError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::AuthenticationRequiredError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::ContentReSendError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::ContentConflictError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::ContentGoneError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::InternalServerError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::OperationNotImplementedError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::ServiceUnavailableError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::ProtocolUnknownError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::ProtocolInvalidOperationError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::UnknownNetworkError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::UnknownProxyError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::UnknownContentError );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::ProtocolFailure );
        INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR( QNetworkReply::UnknownServerError );
        //INVEST_OPENAPI_MODELS_HELPERS_CASE_QNETWORKREPLY_NETWORKERROR(  );
        default : return QString("Unknown QNetworkReply Error: ") + QString::number((unsigned)e);
    };
}

inline
void dumpHttpResponse( const QNetworkReply &reply )
{
    if (!getResponsesDebug())
        return;

    QIODevice * pQIODevice = const_cast<QNetworkReply*>(&reply);

    qDebug().nospace().noquote() << "----- RESPONSE -----";
    qDebug().nospace().noquote() << "Status: " << toString(reply.error()) << ", look https://doc.qt.io/qt-5/qnetworkreply.html#NetworkError-enum for details";
    // https://doc.qt.io/qt-5/qnetworkrequest.html#Attribute-enum
    qDebug().nospace().noquote() << "HTTP Status: " << reply.attribute( QNetworkRequest::HttpStatusCodeAttribute ).toUInt();

    auto rawHeaders = reply.rawHeaderList();
    foreach (auto key, rawHeaders)
    {
        qDebug().nospace().noquote() << key << ": " << reply.rawHeader(key);
    }

    qDebug().nospace().noquote() << "--- BODY";
    qDebug().nospace().noquote() << pQIODevice->readAll();
    //qDebug() << pQIODevice->readAll();
    qDebug().nospace().noquote() << "--- RESPONSE END";
    
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------





//----------------------------------------------------------------------------

} // namespace OpenAPI


