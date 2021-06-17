/*! \file
    \brief 
 */

#pragma once

#include <QDebug>
#include <QSharedPointer>

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
#define INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( completableFutureObjPtr, pApi, apiMethod, httpMethod )                      \
                (completableFutureObjPtr)->connectTo( pApi                                                                            \
                                                    , &std::remove_pointer<decltype(pApi)>::type:: apiMethod ## httpMethod ## Signal  \
                                                    , &std::remove_pointer<decltype(pApi)>::type:: apiMethod ## httpMethod ## SignalE \
                                                    )


#define INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API_GET( completableFutureObjPtr, pApi, apiMethod ) \
                OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( pApi, apiMethod, completableFutureObjPtr, Get )


#define INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API_POST( completableFutureObjPtr, pApi, apiMethod ) \
                OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( pApi, apiMethod, completableFutureObjPtr, Post )

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
namespace invest_openapi
{




//----------------------------------------------------------------------------
template<typename ValueType>
class OpenApiCompletableFuture : public OpenApiCompletableFutureBase
{

public:

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
        connect( pSource, completeSignal, this, qOverload<value_type>(&OpenApiCompletableFutureBase::onComplete), Qt::QueuedConnection );
        connect( pSource, errorSignal   , this, qOverload<value_type, QNetworkReply::NetworkError, QString>(&OpenApiCompletableFutureBase::onError), Qt::QueuedConnection );
    }


protected:

/*
    void complete(const value_type &v)
    {
        value = v;
        m_complete.store(true, std::memory_order_relaxed);
    }
*/

    virtual void onComplete( value_type v) override
    {
        value = v;

        #if defined(INVEST_OPENAPI_DEBUG_DUMP_COMPLETABLE_FUTURE_HANDLER_VALUES)
            qDebug().nospace().noquote() << "!!! (+) " << __FUNCTION_NAME__ << " - v    : " << v.asJson();
            qDebug().nospace().noquote() << "!!! (+) " << __FUNCTION_NAME__ << " - value: " << value.asJson();
        #endif

        m_complete.store(true, std::memory_order_relaxed);
    }

    virtual void onError( value_type v, QNetworkReply::NetworkError et, QString es ) override
    {
        value = v;

        #if defined(INVEST_OPENAPI_DEBUG_DUMP_COMPLETABLE_FUTURE_HANDLER_VALUES)
            qDebug().nospace().noquote() << "!!! (+) " << __FUNCTION_NAME__ << " - v    : " << v.asJson();
            qDebug().nospace().noquote() << "!!! (+) " << __FUNCTION_NAME__ << " - value: " << value.asJson();
        #endif

        errorComplete(et, es);
    }



}; // class OpenApiCompletableFuture

//----------------------------------------------------------------------------





template< typename IteratorType > inline
IteratorType findOpenApiCompletableFutureFinished( IteratorType b, IteratorType e )
{
    pollMessageQueue();

    for(; b!=e; ++b )
    {
        auto &f = *b;
        if (f->isFinished())
            return b;
    }

    return b;
}

template< typename IteratorType > inline
IteratorType findOpenApiCompletableFutureCompleted( IteratorType b, IteratorType e )
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


