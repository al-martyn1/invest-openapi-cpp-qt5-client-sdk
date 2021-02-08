/*! \file
    \brief 
 */

//----------------------------------------------------------------------------
#pragma once

#include <QString>
#include <QSettings>

#include <exception>
#include <stdexcept>

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
namespace invest_openapi
{




//----------------------------------------------------------------------------
struct AuthConfig
{
    QString  token;
    QString  sanboxToken;
    bool     sandboxMode = false;

    QString  brokerAccountId       ;
    QString  sandboxBrokerAccountId;

    QString getToken() const
    {
        return sandboxMode ? sanboxToken : token;
    }

    QString getBrokerAccountId() const
    {
        return sandboxMode ? sandboxBrokerAccountId : brokerAccountId;
    }

    void load( const QSettings &settings )
    {
        token                  = settings.value("token"        ).toString();
        brokerAccountId        = settings.value("broker-account-id").toString();

        sanboxToken            = settings.value("sandbox-token").toString();
        sandboxBrokerAccountId = settings.value("sandbox-broker-account-id" ).toString();  

        sandboxMode            = settings.value("sandbox-mode" ).toBool();
    }

    void checkValid() const
    {
        // if (token.isEmpty())
        //     throw std::runtime_error("Token is empty ('token')");
    }

    AuthConfig( const QSettings &settings )
    {
        load(settings);
        checkValid();
    }

    AuthConfig( const QString &settingsFile )
    {
        QSettings settings(settingsFile, QSettings::IniFormat);
        load(settings);
        checkValid();
    }

    AuthConfig( const QString &tk, const QString &smbxtk, bool sm )
    {
        token       = tk;
        sanboxToken = smbxtk;
        sandboxMode = sm;
    }

};

//----------------------------------------------------------------------------




} // namespace invest_openapi


