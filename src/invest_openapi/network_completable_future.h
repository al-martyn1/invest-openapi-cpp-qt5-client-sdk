#pragma once

#include <type_traits>
#include "network_completable_future_base.h"


/*
    #define SUPER_CONNECT(sender, signal, receiver, slot) \
connect(sender  , &std::remove_pointer<decltype(sender)>::type::signal, \
        receiver, &std::remove_pointer<decltype(receiver)>::type::slot)
*/

#define INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( completableFutureObjPtr, pApi, apiMethod, httpMethod )                      \
                (completableFutureObjPtr)->connectTo( pApi                                                                            \
                                                    , &std::remove_pointer<decltype(pApi)>::type:: apiMethod ## httpMethod ## Signal  \
                                                    , &std::remove_pointer<decltype(pApi)>::type:: apiMethod ## httpMethod ## SignalE \
                                                    )


#define INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API_GET( completableFutureObjPtr, pApi, apiMethod ) \
                OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( pApi, apiMethod, completableFutureObjPtr, Get )


#define INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API_POST( completableFutureObjPtr, pApi, apiMethod ) \
                OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( pApi, apiMethod, completableFutureObjPtr, Post )



namespace invest_openapi
{




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

    virtual void onComplete(value_type v) override
    {
        value = v;
        m_complete.store(true, std::memory_order_relaxed);
    }


}; // class OpenApiCompletableFuture


} // namespace invest_openapi


