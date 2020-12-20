#pragma once

#include <QObject>
#include <QDebug>
#include <QtDebug>
#include <QTest>
#include <QtConcurrent/QtConcurrent>

#include <iostream>
#include <exception>
#include <stdexcept>

#include "models.h"

#include "invest_openapi_safe_main.h"
#include "openapi_completable_future.h"
#include "api_config.h"
#include "auth_config.h"


#if defined(_MSC_VER)
    #pragma comment(lib, "tkf_invest_oa")
#endif

// Invest OpenAPI interface method
// #define IOAIM

#define IOA_ABSTRACT_METHOD = 0
#define IOA_METHOD_IMPL override


namespace invest_openapi
{



inline
void pollMessageQueue()
{
    QTest::qWait(0);
}




struct IOpenApi
{

}; // struct IOpenApi



struct ISanboxOpenApi : public IOpenApi
{

    virtual QSharedPointer<SandboxRegisterResponse>
        sandboxRegister( BrokerAccountType v )
    IOA_ABSTRACT_METHOD;
    
}; // struct ISanboxOpenApi



class OpenApi : public IOpenApi
{

public:

    OpenApi( const ApiConfig  &apiConfig
           , const AuthConfig &authConfig
           )
    : m_apiConfig(apiConfig)
    , m_authConfig(authConfig)
    {}

protected:

    ApiConfig  m_apiConfig;
    AuthConfig m_authConfig;


}; // class OpenApi




class SanboxOpenApi : public OpenApi
                    , public ISanboxOpenApi
{

    virtual QSharedPointer<SandboxRegisterResponse>
        sandboxRegister( BrokerAccountType v )
    IOA_METHOD_IMPL
    {
        SandboxRegisterRequest   sandboxRegisterRequest;
        sandboxRegisterRequest.setBrokerAccountType(v);
  
    }

}; // class SanboxOpenApi











} // namespace invest_openapi

