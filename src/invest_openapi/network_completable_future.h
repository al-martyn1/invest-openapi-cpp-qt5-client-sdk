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

    value_type& join() const
    {
        joinImpl();
        return value;
    }



protected:


    void complete(const value_type &v)
    {
        value = v;
        m_complete.store(true, std::memory_order_relaxed);
    }
    


}; // class NetworkCompletableFuture


} // namespace invest_openapi


