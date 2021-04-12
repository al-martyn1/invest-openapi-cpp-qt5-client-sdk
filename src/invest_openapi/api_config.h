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
struct ApiConfig
{
    QString  url;
    QString  urlSandbox;
    QString  urlStreaming;

    //bool debugRequests ;
    //bool debugResponses;



    void load( const QSettings &settings )
    {
        url          = settings.value("ru.tinkoff.invest.openapi.host").toString();
        urlSandbox   = settings.value("ru.tinkoff.invest.openapi.host-sandbox").toString();
        urlStreaming = settings.value("ru.tinkoff.invest.openapi.streaming").toString();

        // Remove trailing '/' from URLs, if exists

        if (!url.isEmpty() && url.back()=='/')
            url.chop(1);

        if (!urlSandbox.isEmpty() && urlSandbox.back()=='/')
            urlSandbox.chop(1);

        //debugRequests  = settings.value("app.debug.dump.requests" , QVariant(false)).toBool();
        //debugResponses = settings.value("app.debug.dump.responses", QVariant(false)).toBool();

    }

    void checkValid() const
    {
        if (url.isEmpty())
            throw std::runtime_error("API url is empty ('ru.tinkoff.invest.openapi.host')");

        if (urlSandbox.isEmpty())
            throw std::runtime_error("API sandbox url is empty ('ru.tinkoff.invest.openapi.host-sandbox')");

        if (urlStreaming.isEmpty())
            throw std::runtime_error("API streaming url is empty ('ru.tinkoff.invest.openapi.streaming')");
    }

    ApiConfig( const QSettings &settings )
    {
        load(settings);
        checkValid();
    }

    ApiConfig( const QString &settingsFile )
    {
        QSettings settings(settingsFile, QSettings::IniFormat);
        load(settings);
        checkValid();
    }

};

//----------------------------------------------------------------------------



} // namespace invest_openapi


