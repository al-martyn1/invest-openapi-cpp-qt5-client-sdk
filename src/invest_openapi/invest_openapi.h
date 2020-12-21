/*! \file
    \brief 
 */

#pragma once

#include <QObject>
#include <QDebug>
#include <QtDebug>
#include <QTest>
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
#include "factory.h"


#if defined(_MSC_VER)
    #pragma comment(lib, "tkf_invest_oa")
#endif

// Invest OpenAPI interface method
// #define IOAIM

#define TKF_IOA_ABSTRACT_METHOD_END   = 0
#define TKF_IOA_METHOD_IMPL_END       override

#define TKF_IOA_SHARED_COMPLETABLE_FUTURE( T )               QSharedPointer< OpenApiCompletableFuture< T > >
#define TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( T, valName )  \
        TKF_IOA_SHARED_COMPLETABLE_FUTURE( T ) valName = TKF_IOA_SHARED_COMPLETABLE_FUTURE( T )( new OpenApiCompletableFuture< T >() )


#define TKF_IOA_ABSTRACT_METHOD( ret, nameWithParams )     virtual TKF_IOA_SHARED_COMPLETABLE_FUTURE( ret ) nameWithParams TKF_IOA_ABSTRACT_METHOD_END
#define TKF_IOA_METHOD_IMPL( ret, nameWithParams )         virtual TKF_IOA_SHARED_COMPLETABLE_FUTURE( ret ) nameWithParams TKF_IOA_METHOD_IMPL_END


namespace invest_openapi
{



inline
void pollMessageQueue()
{
    QTest::qWait(0);
}



class OpenApiFactory;


struct IOpenApi
{
    virtual void    setBrokerAccountId( const QString &id ) = 0;
    virtual QString getBrokerAccountId()                    = 0;

    virtual ~IOpenApi() {};
}; // struct IOpenApi



struct ISanboxOpenApi : public IOpenApi
{
    TKF_IOA_ABSTRACT_METHOD( SandboxRegisterResponse, sandboxRegister(BrokerAccountType v) );
    TKF_IOA_ABSTRACT_METHOD( SandboxRegisterResponse, sandboxRegister(BrokerAccountType::eBrokerAccountType v) );
    //TKF_IOA_ABSTRACT_METHOD( Empty, sandboxClear(QString broker_account_id = QString() ) );
    //TKF_IOA_ABSTRACT_METHOD( Empty, sandboxClear(double balance, QString broker_account_id = QString() ) );
    //SandboxSetPositionBalanceRequest

    TKF_IOA_ABSTRACT_METHOD( Empty, sandboxCurrenciesBalanceSet(const SandboxCurrency::eSandboxCurrency currency, double balance, QString broker_account_id = QString() ) );
    

/*
    TKF_IOA_ABSTRACT_METHOD( Empty, sandboxCurrenciesBalance() );
    TKF_IOA_ABSTRACT_METHOD( Empty, () );
    TKF_IOA_ABSTRACT_METHOD( Empty, () );
    TKF_IOA_ABSTRACT_METHOD( Empty, () );
    TKF_IOA_ABSTRACT_METHOD( Empty, () );

    void sandboxCurrenciesBalancePost(const SandboxSetCurrencyBalanceRequest &sandbox_set_currency_balance_request, const QString &broker_account_id);
    void sandboxPositionsBalancePost(const SandboxSetPositionBalanceRequest &sandbox_set_position_balance_request, const QString &broker_account_id);
    void sandboxRegisterPost(const SandboxRegisterRequest &sandbox_register_request);
    void sandboxRemovePost(const QString &broker_account_id);

    void sandboxCurrenciesBalancePostSignal(Empty summary);
    void sandboxPositionsBalancePostSignal(Empty summary);
    void sandboxRegisterPostSignal(SandboxRegisterResponse summary);
    void sandboxRemovePostSignal(Empty summary);
*/

    
}; // struct ISanboxOpenApi



class OpenApiImpl : public IOpenApi
{

    friend class OpenApiFactory;

public:

    OpenApiImpl( const ApiConfig  &apiConfig
               , const AuthConfig &authConfig
               )
    : m_apiConfig(apiConfig)
    , m_authConfig(authConfig)
    {
        initApis(OpenApiFactory(m_apiConfig, m_authConfig));
    }

    virtual void    setBrokerAccountId( const QString &id ) override
    {
        m_brokerAccountId = id;
    }

    virtual QString getBrokerAccountId()                    override
    {
        return m_brokerAccountId;
    }


protected:

    void initApis( const OpenApiFactory &factory )
    {
        m_pOrdersApi     = factory.getApiImpl< OrdersApi     >();
        m_pPortfolioApi  = factory.getApiImpl< PortfolioApi  >();
        m_pMarketApi     = factory.getApiImpl< MarketApi     >();
        m_pOperationsApi = factory.getApiImpl< OperationsApi >();
        m_pUserApi       = factory.getApiImpl< UserApi       >();
    }


    ApiConfig  m_apiConfig;
    AuthConfig m_authConfig;

    QSharedPointer<OrdersApi    >  m_pOrdersApi    ;
    QSharedPointer<PortfolioApi >  m_pPortfolioApi ;
    QSharedPointer<MarketApi    >  m_pMarketApi    ;
    QSharedPointer<OperationsApi>  m_pOperationsApi;
    QSharedPointer<UserApi      >  m_pUserApi      ;

    QString                        m_brokerAccountId;


}; // class OpenApiImpl




class SanboxOpenApiImpl : public OpenApiImpl
                        , public ISanboxOpenApi
{

    friend class OpenApiFactory;

    void checkBrokerAccountIdParam( QString &id )
    {
        if (id.isEmpty())
        {
            if (m_brokerAccountId.isEmpty())
                throw std::runtime_error("Broker accoubt ID not set");
            id = m_brokerAccountId;
        }
    }

public:
    
    SanboxOpenApiImpl( const ApiConfig  &apiConfig
                     , const AuthConfig &authConfig
                     )
    : OpenApiImpl( apiConfig, authConfig )
    {
        OpenApiFactory factory = OpenApiFactory(m_apiConfig, m_authConfig);
        m_pSandboxApi = factory.getApiImpl< SandboxApi >();
    }

    virtual void    setBrokerAccountId( const QString &id ) override
    {
        OpenApiImpl::setBrokerAccountId(id);
    }

    virtual QString getBrokerAccountId()                    override
    {
        return OpenApiImpl::getBrokerAccountId();
    }



    TKF_IOA_METHOD_IMPL( SandboxRegisterResponse, sandboxRegister(BrokerAccountType v) )
    {
        SandboxRegisterRequest sandboxRegisterRequest;
        sandboxRegisterRequest.setBrokerAccountType(v);

        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( SandboxRegisterResponse, sandboxRegisterResponse );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( sandboxRegisterResponse.get(), m_pSandboxApi.get(), sandboxRegister, Post );
        m_pSandboxApi->sandboxRegisterPost(sandboxRegisterRequest);

        return sandboxRegisterResponse;
    }

    TKF_IOA_METHOD_IMPL( SandboxRegisterResponse, sandboxRegister(BrokerAccountType::eBrokerAccountType v) )
    {
        BrokerAccountType brokerAccountType;
        brokerAccountType.setValue( v );
        return sandboxRegister(brokerAccountType);
    }
/*
    TKF_IOA_METHOD_IMPL( Empty, sandboxClear(QString broker_account_id = QString() ) )
    {
        checkBrokerAccountIdParam(broker_account_id);


    
    }

    TKF_IOA_ABSTRACT_METHOD( Empty, sandboxClear(double balance, QString broker_account_id = QString() ) );
*/

    //TKF_IOA_METHOD_IMPL( Empty, sandboxClear(const QString &figi, double balance, QString broker_account_id = QString() ) )
    TKF_IOA_METHOD_IMPL( Empty, sandboxCurrenciesBalanceSet(const SandboxCurrency::eSandboxCurrency currency, double balance, QString broker_account_id = QString() ) )
    {
        checkBrokerAccountIdParam(broker_account_id);

        SandboxSetCurrencyBalanceRequest request;

        SandboxCurrency sandboxCurrency;
        sandboxCurrency.setValue(currency);

        request.setCurrency(sandboxCurrency);
        request.setBalance(balance);

        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( Empty, response );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( response.get(), m_pSandboxApi.get(), sandboxCurrenciesBalance, Post );
        m_pSandboxApi->sandboxCurrenciesBalancePost(request, broker_account_id);
        
        return response;
    }


/*
    SandboxCurrency getCurrency() const;
    void setCurrency(const SandboxCurrency &currency);
    bool is_currency_Set() const;
    bool is_currency_Valid() const;

    double getBalance() const;
    void setBalance(const double &balance);




    void sandboxClearPost(const QString &broker_account_id);
    void sandboxCurrenciesBalancePost(const SandboxSetCurrencyBalanceRequest &sandbox_set_currency_balance_request, const QString &broker_account_id);
    void sandboxPositionsBalancePost(const SandboxSetPositionBalanceRequest &sandbox_set_position_balance_request, const QString &broker_account_id);
    void sandboxRegisterPost(const SandboxRegisterRequest &sandbox_register_request);
    void sandboxRemovePost(const QString &broker_account_id);

    void sandboxClearPostSignal(Empty summary);
    void sandboxCurrenciesBalancePostSignal(Empty summary);
    void sandboxPositionsBalancePostSignal(Empty summary);
    void sandboxRegisterPostSignal(SandboxRegisterResponse summary);
    void sandboxRemovePostSignal(Empty summary);
*/

protected:

    QSharedPointer<SandboxApi>  m_pSandboxApi;


}; // class SanboxOpenApiImpl


inline
QSharedPointer<IOpenApi>
createOpenApi( const ApiConfig  &apiConfig
             , const AuthConfig &authConfig
             )
{
    OpenApiFactory faq = OpenApiFactory(apiConfig,authConfig);

    IOpenApi *pApi = authConfig.sandboxMode
                   ? static_cast<IOpenApi*>(static_cast<OpenApiImpl*>(new SanboxOpenApiImpl(apiConfig,authConfig)))
                   : static_cast<IOpenApi*>(new OpenApiImpl(apiConfig,authConfig))
                   ;

    return QSharedPointer<IOpenApi>( pApi );
}







} // namespace invest_openapi

