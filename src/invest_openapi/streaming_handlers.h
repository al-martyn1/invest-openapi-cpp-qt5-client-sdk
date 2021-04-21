/*! \file
    \brief 
 */

#pragma once

#include <QObject>
#include <QDebug>
#include <QtDebug>
#include <QTest>
#include <QTimeZone>

//#include <QtConcurrent/QtConcurrent>

#include <iostream>
#include <exception>
#include <stdexcept>

#include "models.h"

#include "invest_openapi_safe_main.h"
#include "openapi_completable_future_base.h"
#include "openapi_completable_future.h"
#include "api_config.h"
#include "auth_config.h"
#include "logging_config.h"
#include "currencies_config.h"
#include "factory.h"
#include "utility.h"
#include "qdebug_support.h"
#include "marty_decimal.h"
#include "qt_time_helpers.h"



//----------------------------------------------------------------------------
namespace invest_openapi
{



//----------------------------------------------------------------------------
struct IStreamingEventHandler
{
    virtual ~IStreamingEventHandler() {};

    virtual void handleEvent( const QString &eventId, const QString &eventData ) = 0;


}; // struct IStreamingEventHandler

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
struct BasicStreamingErrorHandler : public IStreamingEventHandler
{

    virtual void handleError( const OpenAPI::StreamingError &err )
    {
        qDebug() << "Streaming error: " << err.getPayload().getMessage();
        qDebug() << "             at: " << err.getTime();
    }

    virtual void handleEvent( const QString &eventId, const QString &eventData ) override
    {
        OpenAPI::StreamingError data;
        data.fromJson(eventData);
        handleError(data);
    }


}; // struct BasicStreamingErrorHandler

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
struct BasicStreamingCandleResponseHandler : public IStreamingEventHandler
{

    virtual void handleCandleResponse( const OpenAPI::StreamingCandleResponse &candleResponse ) = 0;

    virtual void handleEvent( const QString &eventId, const QString &eventData ) override
    {
        OpenAPI::StreamingCandleResponse data;
        data.fromJson(eventData);
        handleCandleResponse(data);
    }

}; // struct BasicStreamingCandleResponseHandler

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
struct BasicStreamingMarketInstrumentResponseHandler : public IStreamingEventHandler
{

    virtual void handleMarketInstrumentResponse( const OpenAPI::StreamingMarketInstrumentResponse &marketInstrumentResponse ) = 0;

    virtual void handleEvent( const QString &eventId, const QString &eventData ) override
    {
        OpenAPI::StreamingMarketInstrumentResponse data;
        data.fromJson(eventData);
        handleMarketInstrumentResponse(data);
    }

}; // struct BasicStreamingMarketInstrumentResponseHandler

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
struct BasicStreamingOrderbookResponseHandler : public IStreamingEventHandler
{

    virtual void handleOrderbookResponse( const OpenAPI::StreamingOrderbookResponse &orderbookResponse ) = 0;

    virtual void handleEvent( const QString &eventId, const QString &eventData ) override
    {
        OpenAPI::StreamingOrderbookResponse data;
        data.fromJson(eventData);
        handleOrderbookResponse(data);
    }

}; // struct BasicStreamingOrderbookResponseHandler

//----------------------------------------------------------------------------






//----------------------------------------------------------------------------


} // namespace invest_openapi

