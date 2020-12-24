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
inline
void dumpHttpRequestInput( const HttpRequestInput &input )
{
    if (!getRequestsDebug())
        return;

    // default mode is space() and quote()

    qDebug().nospace().noquote() << "--- REQUEST";

    qDebug().nospace().noquote() << input.http_method << " " << input.url_str;

    foreach (QString key, input.headers.keys())
    {
        qDebug().nospace().noquote() << key << ": " << input.headers.value(key);
    }

    qDebug().nospace().noquote() << "\n";

    qDebug().nospace().noquote() << input.request_body;

    qDebug().nospace().noquote() << "--- EOR";
    qDebug().nospace().noquote() << "";

}

//----------------------------------------------------------------------------
inline
void dumpHttpRequestInput( const HttpRequestInput &input, const QNetworkRequest &request )
{
    if (!getRequestsDebug())
        return;

    qDebug().nospace().noquote() << Qt::endl;
    qDebug().nospace().noquote() << "--- REQUEST";

    qDebug().nospace().noquote() << input.http_method << " " << input.url_str;
    //qDebug().nospace().noquote() << request.url().toString();

    auto headers = request.rawHeaderList();
    foreach (auto key, headers)
    {
        qDebug().nospace().noquote() << key << ": " << request.rawHeader(key);
    }

    qDebug().nospace().noquote() << Qt::endl;
    qDebug().nospace().noquote() << "--- BODY";

    qDebug().nospace().noquote() << input.request_body;

    qDebug().nospace().noquote() << "--- EOR";
    qDebug().nospace().noquote() << Qt::endl;

}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------

} // namespace OpenAPI


