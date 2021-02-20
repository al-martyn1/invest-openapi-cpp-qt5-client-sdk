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
struct BalanceConfig
{
    QStringList   figis;

    void load( const QSettings &settings )
    {
        figis = settings.value("instruments" ).toStringList();
    }

    void checkValid() const
    {
        // if (token.isEmpty())
        //     throw std::runtime_error("Token is empty ('token')");
    }

    BalanceConfig( const QSettings &settings )
    {
        load(settings);
        checkValid();
    }

    BalanceConfig( const QString &settingsFile )
    {
        QSettings settings(settingsFile, QSettings::IniFormat);
        load(settings);
        checkValid();
    }


};

//----------------------------------------------------------------------------




} // namespace invest_openapi


