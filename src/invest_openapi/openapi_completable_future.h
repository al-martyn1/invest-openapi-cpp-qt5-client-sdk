/*! \file
    \brief 
 */

#pragma once

#include <QDebug>
#include <QSharedPointer>

#include <iostream>
#include <iomanip>
#include <exception>
#include <stdexcept>

#include <type_traits>
#include "openapi_completable_future_base.h"
#include "func.h"
#include "lib_config.h"


/*
    #define SUPER_CONNECT(sender, signal, receiver, slot) \
connect(sender  , &std::remove_pointer<decltype(sender)>::type::signal, \
        receiver, &std::remove_pointer<decltype(receiver)>::type::slot)
*/

//----------------------------------------------------------------------------
#define INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( completableFutureObjPtr, pApi, apiMethod, httpMethod )                          \
                (completableFutureObjPtr)->connectTo( pApi                                                                                \
                                                    , &std::remove_pointer<decltype(pApi)>::type:: apiMethod ## httpMethod ## SignalFull  \
                                                    , &std::remove_pointer<decltype(pApi)>::type:: apiMethod ## httpMethod ## SignalEFull \
                                                    )


#define INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API_GET( completableFutureObjPtr, pApi, apiMethod ) \
                OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( pApi, apiMethod, completableFutureObjPtr, Get )


#define INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API_POST( completableFutureObjPtr, pApi, apiMethod ) \
                OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( pApi, apiMethod, completableFutureObjPtr, Post )

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
namespace invest_openapi
{



class OpenApiImpl;      
class SanboxOpenApiImpl;



//----------------------------------------------------------------------------
template<typename ValueType>
class OpenApiCompletableFuture : public OpenApiCompletableFutureBase
{

public:

    friend class OpenApiImpl;      
    friend class SanboxOpenApiImpl;


    typedef ValueType value_type;

    value_type  value; //!< Do not touch 'value' while isCompleted() returns false

    OpenApiCompletableFuture<ValueType>& join() const
    {
        joinImpl();

        return const_cast< OpenApiCompletableFuture<ValueType>& >(*this);
    }


    value_type result() const // QFuture compatible
    {
        return value;
    }


    template< typename SignalSourseObjectType, typename SignalCompleteType, typename SignalErrorType >
    void connectTo( SignalSourseObjectType * pSource
                  , SignalCompleteType completeSignal
                  , SignalErrorType    errorSignal
                  )
    {
        // https://wiki.qt.io/New_Signal_Slot_Syntax

        slotConnectionComplete = connect( pSource, completeSignal, this
                                        , qOverload<OpenAPI::HttpRequestWorker*, value_type>(&OpenApiCompletableFutureBase::onComplete), Qt::QueuedConnection 
                                        );

        slotConnectionError    = connect( pSource, errorSignal   , this
                                        , qOverload<OpenAPI::HttpRequestWorker*, QNetworkReply::NetworkError, QString>(&OpenApiCompletableFutureBase::onError), Qt::QueuedConnection
                                        );
    }


protected:


/*
    void complete(const value_type &v)
    {
        value = v;
        m_complete.store(true, std::memory_order_relaxed);
    }
*/

    virtual void onComplete( OpenAPI::HttpRequestWorker *worker, value_type v) override
    {
        if (m_pWorker==0)
        {
            throw std::runtime_error( std::string("__FUNCTION__") + ": worker is not set" );

            // __FUNCTION__, __func__ - https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros?view=msvc-160
            // https://stackoverflow.com/questions/15305310/predefined-macros-for-function-name-func
        }

        if (m_pWorker!=worker)
            return;

        value = v;

        #if defined(INVEST_OPENAPI_DEBUG_DUMP_COMPLETABLE_FUTURE_HANDLER_VALUES)
            qDebug().nospace().noquote() << "!!! (+) " << __FUNCTION_NAME__ << " - v    : " << v.asJson();
            qDebug().nospace().noquote() << "!!! (+) " << __FUNCTION_NAME__ << " - value: " << value.asJson();
        #endif

        //std::cout << "Got response, obj : " << (void*)this << ", value: " << value.asJson() << std::endl;

        onCompleteCommon();
    }

    /*
    virtual void onError( HttpRequestWorker *worker, QNetworkReply::NetworkError et, QString es ) override
    {
        // value = v;

        #if defined(INVEST_OPENAPI_DEBUG_DUMP_COMPLETABLE_FUTURE_HANDLER_VALUES)
            // qDebug().nospace().noquote() << "!!! (+) " << __FUNCTION_NAME__ << " - v    : " << v.asJson();
            // qDebug().nospace().noquote() << "!!! (+) " << __FUNCTION_NAME__ << " - value: " << value.asJson();
        #endif

        errorComplete(et, es);
    }
    */


}; // class OpenApiCompletableFuture

//----------------------------------------------------------------------------





template< typename IteratorType, typename IsFinishedPred > inline
IteratorType findOpenApiCompletableFutureFinished( IteratorType b, IteratorType e, IsFinishedPred isFinishedPred )
{
    pollMessageQueue();

    for(; b!=e; ++b )
    {
        //auto &f = *b;
        //if (f->isFinished())
        if (isFinishedPred(b))
            return b;
    }

    return b;
}

template< typename IteratorType, typename IsFinishedPred > inline
IteratorType findOpenApiCompletableFutureCompleted( IteratorType b, IteratorType e, IsFinishedPred isFinishedPred )
{
    return findOpenApiCompletableFutureFinished(b,e);
}



//----------------------------------------------------------------------------
template<typename T>
inline
void dump( const QSharedPointer< OpenApiCompletableFuture< T > > &val )
{
    qDebug().nospace().noquote() << val->value.asJson();
}

template<typename T>
inline
void dumpIfError( const QSharedPointer< OpenApiCompletableFuture< T > > &val )
{
    if (!val->isCompletionError())
        return;

    qDebug().nospace().noquote() << val->getErrorMessage();
    dump(val);
}

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------




} // namespace invest_openapi


