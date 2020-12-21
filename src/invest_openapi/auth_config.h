/*! \file
    \brief 
 */

#pragma once

#include <QString>
#include <QSettings>

#include <exception>
#include <stdexcept>


namespace invest_openapi
{


struct AuthConfig
{
    QString  token;
    bool     sandboxMode = false;


    void load( const QSettings &settings )
    {
        token        = settings.value("token").toString();
        sandboxMode  = settings.value("sandbox-mode").toBool();
    }

    void checkValid() const
    {
        if (token.isEmpty())
            throw std::runtime_error("Token is empty ('token')");
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

    AuthConfig( const QString &tk, bool sm )
    {
        token       = tk;
        sandboxMode = sm;
    }


};


} // namespace invest_openapi

