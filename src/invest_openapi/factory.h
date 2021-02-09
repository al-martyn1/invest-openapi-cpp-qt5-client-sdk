/*! \file
    \brief 
 */

#pragma once

#include <QString>
#include <QUrl>

#include <exception>
#include <stdexcept>

#include "models.h"


namespace invest_openapi
{


class OpenApiImpl;
class SanboxOpenApiImpl;


class OpenApiFactory
{

    friend class OpenApiImpl;
    friend class SanboxOpenApiImpl;

public:


    OpenApiFactory( const ApiConfig &apiConfig, const QString &token, const QString &sandboxToken, bool sandboxMode )
    : m_apiConfig(apiConfig)
    , m_authConfig( token, sandboxToken, sandboxMode )
    {
        m_apiConfig.checkValid();
        m_authConfig.checkValid();
    }

    OpenApiFactory( const ApiConfig &apiConfig, const AuthConfig &authConfig )
    : m_apiConfig(apiConfig)
    , m_authConfig(authConfig)
    {
        m_apiConfig.checkValid();
        m_authConfig.checkValid();
    }

    OpenApiFactory( const OpenApiFactory &f )
    : m_apiConfig(f.m_apiConfig)
    , m_authConfig(f.m_authConfig)
    {
        m_apiConfig.checkValid();
        m_authConfig.checkValid();
    }

#if !defined(TEST_MODE)
protected:
#endif

    template< typename ApiType >
    QSharedPointer<ApiType>
    getApiImpl( const int timeOut = 0 ) const
    {
        QUrl apiUrl = getApiUrl();

        ApiType *pApi = new ApiType( apiUrl.scheme(), apiUrl.host(), apiUrl.port(0), apiUrl.path(), timeOut );

        if (!pApi) //-V668
        {
            throw std::runtime_error("Something goes wrong");
            return QSharedPointer<ApiType>(pApi);
        }

        pApi->addHeaders( "Authorization", QString("Bearer ") + m_authConfig.getToken() );
        pApi->addHeaders( "Accept", "application/json" );

        return QSharedPointer<ApiType>(pApi);
    }

#if !defined(IOA_NO_OBSOLETE_METHODS_SANDBOX_MARKET_API)

// Obsolete 

    SandboxApi* getSandboxApi(const int timeOut = 0) const
    {
        //if (!m_sandboxMode)
        //    throw std::runtime_error("Attempting to use the sandbox in combat mode");

        if (!isSandboxMode())
            return 0;
        
        QUrl apiUrl = getApiUrl();

        SandboxApi *pApi = new SandboxApi( apiUrl.scheme(), apiUrl.host(), apiUrl.port(0), apiUrl.path(), timeOut );
        if (!pApi) //-V668
            return pApi;

        pApi->addHeaders( "Authorization", QString("Bearer ") + m_authConfig.getToken() );
        pApi->addHeaders( "Accept", "application/json" );

        return pApi;
    }

    MarketApi* getMarketApi(const int timeOut = 0) const
    {
        QUrl apiUrl = getApiUrl();

        MarketApi *pApi = new MarketApi( apiUrl.scheme(), apiUrl.host(), apiUrl.port(0), apiUrl.path(), timeOut );

        if (!pApi) //-V668
            return pApi;

        pApi->addHeaders( "Authorization", QString("Bearer ") + m_authConfig.getToken() );
        pApi->addHeaders( "Accept", "application/json" );
        
        return pApi;
    }

    bool isSandboxMode() const
    {
        return m_authConfig.sandboxMode;
    }

// End of obsolete
#endif



protected:

    QUrl getApiUrl() const
    {
        if (!isSandboxMode())
            return QUrl::fromUserInput( m_apiConfig.url );
        else
            return QUrl::fromUserInput( m_apiConfig.urlSandbox );
    }

    ApiConfig    m_apiConfig;
    AuthConfig   m_authConfig;


}; // class OpenApiFactory



} // namespace invest_openapi

