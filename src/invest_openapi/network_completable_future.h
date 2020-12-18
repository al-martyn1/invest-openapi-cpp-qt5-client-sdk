#pragma once

#include "network_completable_future_base.h"

namespace invest_openapi
{




template<typename ValueType>
class NetworkCompletableFuture : public NetworkCompletableFutureBase
{

public:

    typedef ValueType value_type;

    value_type  value; //!< Do not touch 'value' while isCompleted() returns false

    NetworkCompletableFuture<ValueType>& join() const
    {
        joinImpl();

        return const_cast< NetworkCompletableFuture<ValueType>& >(*this);
    }


    // void sandboxRegisterPostSignal(SandboxRegisterResponse summary);
    // void sandboxRegisterPostSignalE(SandboxRegisterResponse summary, QNetworkReply::NetworkError error_type, QString error_str);
    // typedef void (A::*Ptr) ();
    /*
    template< typename SignalSourseObjectType >
    void connectTo( SignalSourseObjectType * pSource
                  , void (SignalSourseObjectType::*completeSignal)( ValueType v )
                  , void (SignalSourseObjectType::*errorSignal)( ValueType v, QNetworkReply::NetworkError error_type, QString error_str )
                  )
    */
/*
    #define SUPER_CONNECT(sender, signal, receiver, slot) \
connect(sender  , &std::remove_pointer<decltype(sender)>::type::signal, \
        receiver, &std::remove_pointer<decltype(receiver)>::type::slot)
*/

    template< typename SignalSourseObjectType, typename SignalCompleteType, typename SignalErrorType >
    void connectTo( SignalSourseObjectType * pSource
                  , SignalCompleteType completeSignal
                  , SignalErrorType    errorSignal
                  )
    {
        connect( pSource, completeSignal, this, qOverload<value_type>(&NetworkCompletableFutureBase::onComplete) );
        connect( pSource, errorSignal   , this, qOverload<value_type, QNetworkReply::NetworkError, QString>(&NetworkCompletableFutureBase::onError) );
        //SUPER_CONNECT(pSource, completeSignal, this, &NetworkCompletableFutureBase::onComplete);
        //SUPER_CONNECT(pSource, errorSignal   , this, &NetworkCompletableFutureBase::onError);
    }


protected:


    void complete(const value_type &v)
    {
        value = v;
        m_complete.store(true, std::memory_order_relaxed);
    }
    


}; // class NetworkCompletableFuture


} // namespace invest_openapi


