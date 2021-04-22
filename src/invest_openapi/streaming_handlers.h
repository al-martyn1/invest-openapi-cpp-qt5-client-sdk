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
#include <memory>
#include <map>

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
struct BasicStreamingInstrumentInfoResponseHandler : public IStreamingEventHandler
{

    virtual void handleInstrumentInfoResponse( const OpenAPI::StreamingInstrumentInfoResponse &instrumentInfoResponse ) = 0;

    virtual void handleEvent( const QString &eventId, const QString &eventData ) override
    {
        OpenAPI::StreamingInstrumentInfoResponse data;
        data.fromJson(eventData);
        handleInstrumentInfoResponse(data);
    }

}; // struct BasicStreamingInstrumentInfoResponseHandler

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
class StreamingResponseDispatcher
{

public:

    typedef std::shared_ptr< IStreamingEventHandler >     handler_smart_ptr;
    typedef std::map< QString, handler_smart_ptr >        handlers_map_type;


protected:

    handlers_map_type     m_handlers;


public:


    //!
    /*
        //NOTE: pHandler must be dynamically allocated
     */

    void addHandler( const QString &eventId, IStreamingEventHandler *pHandler )
    {
        m_handlers[eventId] = handler_smart_ptr(pHandler);
    }

    void addHandler( const QString &eventId, handler_smart_ptr pHandler )
    {
        m_handlers[eventId] = pHandler;
    }

    void removeHandler( const QString &eventId )
    {
        handlers_map_type::iterator it = m_handlers.find(eventId);
        if (it == m_handlers.end())
            return;

        m_handlers.erase(it);
    }


    //! Dispatches streaming event data to appropriate handler/
    /* \returns true if event handled
     */
    bool dispatchStreamingEvent( const QString &streamingData, const QString &eventId ) const
    {
        handlers_map_type::const_iterator it = m_handlers.find(eventId);
        if (it == m_handlers.end())
            return false;
    
        handler_smart_ptr handlerPtr = it->second;

        handlerPtr->handleEvent( eventId, streamingData );

        return true;
    }


    bool dispatchStreamingEvent( const QString &streamingData ) const
    {
        OpenAPI::GenericStreamingResponse genericStreamingResponse;
        genericStreamingResponse.fromJson(streamingData);

        return dispatchStreamingEvent( streamingData, genericStreamingResponse.getEvent() );
    }


}; // class StreamingResponseDispatcher





//----------------------------------------------------------------------------
/*
template< typename HandlerType >
struct LambdaDrivenStreamingErrorHandler : public BasicStreamingErrorHandler
{

protected:

    HandlerType     handler;


public:

    LambdaDrivenStreamingErrorHandler( const HandlerType &h ) : handler(h) {}

    virtual void handleError( const OpenAPI::StreamingError &err ) override
    {
        handler(err);
    }

};

//---

template< typename HandlerType >
struct ParamHandlerTypeName : public ParamBaseClass
{

protected:

    HandlerType     handler;


public:

    ParamHandlerTypeName( const HandlerType &h ) : handler(h) {}

    virtual void ParamHandlerMemberHandler( const OpenAPI :: ParamHandlerHadledType &streamingObj ) override
    {
        handler(err);
    }

};


template< typename RealHandlerType >
inline
std::shared_ptr< IStreamingEventHandler > make ## ParamHandlerTypeName( const RealHandlerType &h )
{
    return std::make_shared< ParamHandlerTypeName >( h );
}


*/

#define TKF_IOA_DECLARE_SIMPLE_STREAMING_HANDLER_IMPL( ParamHandlerTypeName, ParamBaseClass, ParamHandlerMemberHandler, ParamHandlerHadledType ) \
                                                                                                    \
template< typename RealHandlerType >                                                                \
struct ParamHandlerTypeName : public ParamBaseClass                                                 \
{                                                                                                   \
                                                                                                    \
protected:                                                                                          \
                                                                                                    \
    RealHandlerType     handler;                                                                    \
                                                                                                    \
                                                                                                    \
public:                                                                                             \
                                                                                                    \
    ParamHandlerTypeName( const RealHandlerType &h ) : handler(h) {}                                \
                                                                                                    \
    virtual void                                                                                    \
    ParamHandlerMemberHandler( const OpenAPI :: ParamHandlerHadledType &streamingObj )              \
    override                                                                                        \
    {                                                                                               \
        handler(streamingObj);                                                                      \
    }                                                                                               \
                                                                                                    \
};                                                                                                  \
                                                                                                    \
template< typename RealHandlerType >                                                                \
inline                                                                                              \
std::shared_ptr< IStreamingEventHandler > make ## ParamHandlerTypeName( const RealHandlerType &h )  \
{                                                                                                   \
    std::shared_ptr< ParamHandlerTypeName< RealHandlerType > > ptr = std::make_shared< ParamHandlerTypeName< RealHandlerType > >( h ); \
                                                                                                               \
    std::shared_ptr< IStreamingEventHandler > resPtr = std::dynamic_pointer_cast<IStreamingEventHandler>(ptr); \
                                                                                                               \
    return resPtr;                                                                                             \
}


TKF_IOA_DECLARE_SIMPLE_STREAMING_HANDLER_IMPL( SimpleStreamingErrorHandler                    , BasicStreamingErrorHandler                    , handleError                    , StreamingError                    )
TKF_IOA_DECLARE_SIMPLE_STREAMING_HANDLER_IMPL( SimpleStreamingCandleResponseHandler           , BasicStreamingCandleResponseHandler           , handleCandleResponse           , StreamingCandleResponse           )
TKF_IOA_DECLARE_SIMPLE_STREAMING_HANDLER_IMPL( SimpleStreamingInstrumentInfoResponseHandler   , BasicStreamingInstrumentInfoResponseHandler   , handleInstrumentInfoResponse   , StreamingInstrumentInfoResponse   )
TKF_IOA_DECLARE_SIMPLE_STREAMING_HANDLER_IMPL( SimpleStreamingOrderbookResponseHandler        , BasicStreamingOrderbookResponseHandler        , handleOrderbookResponse        , StreamingOrderbookResponse        )



//----------------------------------------------------------------------------


} // namespace invest_openapi

