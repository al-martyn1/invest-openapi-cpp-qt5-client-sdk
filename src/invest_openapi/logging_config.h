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
struct LoggingConfig
{
    bool debugRequests ;
    bool debugResponses;



    void load( const QSettings &settings )
    {
        debugRequests  = settings.value("logging.debug.dump.requests" , QVariant(false)).toBool();
        debugResponses = settings.value("logging.debug.dump.responses", QVariant(false)).toBool();
    }

    void checkValid() const
    {
    }

    LoggingConfig( const QSettings &settings )
    {
        load(settings);
        checkValid();
    }

    LoggingConfig( const QString &settingsFile )
    {
        QSettings settings(settingsFile, QSettings::IniFormat);
        load(settings);
        checkValid();
    }

};

//----------------------------------------------------------------------------



} // namespace invest_openapi


