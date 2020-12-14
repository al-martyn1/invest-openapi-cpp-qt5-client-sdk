#pragma once

#include <QString>
#include <QSettings>

#include <exception>
#include <stdexcept>


namespace invest_openapi
{


struct ApiConfig
{
    QString  url;
    QString  urlSandbox;
    QString  urlStreaming;


    void load( const QSettings &settings )
    {
        url          = settings.value("ru.tinkoff.invest.openapi.host").toString();
        urlSandbox   = settings.value("ru.tinkoff.invest.openapi.host-sandbox").toString();
        urlStreaming = settings.value("ru.tinkoff.invest.openapi.streaming").toString();
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


} // namespace invest_openapi

