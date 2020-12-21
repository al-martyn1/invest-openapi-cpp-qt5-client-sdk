/*! \file
    \brief 
 */

#pragma once

#include <QString>
#include <QSettings>
#include <QVector>
#include <QStringList>

#include <exception>
#include <stdexcept>

#include "models.h"

#include "utility.h"

namespace invest_openapi
{

class CurrencyConfig
{

public:

    CurrencyConfig()
    : m_isValid(false)
    , m_value(0.0)
    {}

    inline void fromQVariant( const QVariant qv )
    {
        if (qv.isValid() && !qv.isNull())
        {
            m_isValid = true;
            m_value   = qv.toDouble();
        }
        else
        {
            m_isValid = false;
        }
    }

    bool isValid() const
    {
        return m_isValid;
    }

    double getValue() const
    {
        return m_value;
    }

    double value() const
    {
        return m_value;
    }


protected:

    bool     m_isValid;
    double   m_value;

}; // class CurrencyConfig





class CurrenciesConfig
{

protected:

    CurrencyConfig  RUB;
    CurrencyConfig  USD;
    CurrencyConfig  EUR;
    CurrencyConfig  GBP;
    CurrencyConfig  HKD;
    CurrencyConfig  CHF;
    CurrencyConfig  JPY;
    CurrencyConfig  CNY;
    CurrencyConfig  TRY;


    void load( const QSettings &settings )
    {
        RUB.fromQVariant(settings.value("sandbox.currency.RUB"));
        USD.fromQVariant(settings.value("sandbox.currency.USD"));
        EUR.fromQVariant(settings.value("sandbox.currency.EUR"));
        GBP.fromQVariant(settings.value("sandbox.currency.GBP"));
        HKD.fromQVariant(settings.value("sandbox.currency.HKD"));
        CHF.fromQVariant(settings.value("sandbox.currency.CHF"));
        JPY.fromQVariant(settings.value("sandbox.currency.JPY"));
        CNY.fromQVariant(settings.value("sandbox.currency.CNY"));
        TRY.fromQVariant(settings.value("sandbox.currency.TRY"));
    }


public:

    CurrenciesConfig() {}

    CurrencyConfig getCurrenciesConfig( const QString &currency ) const
    {
             if (currency=="RUB") return RUB;
        else if (currency=="USD") return USD;
        else if (currency=="EUR") return EUR;
        else if (currency=="GBP") return GBP;
        else if (currency=="HKD") return HKD;
        else if (currency=="CHF") return CHF;
        else if (currency=="JPY") return JPY;
        else if (currency=="CNY") return CNY;
        else if (currency=="TRY") return TRY;
        else 
        {
            throw std::runtime_error("Unknown CURRENCY value");
        }
    }

    CurrencyConfig getCurrenciesConfig( const SandboxCurrency::eSandboxCurrency &currency ) const
    {
             if (currency==SandboxCurrency::eSandboxCurrency::RUB) return RUB;
        else if (currency==SandboxCurrency::eSandboxCurrency::USD) return USD;
        else if (currency==SandboxCurrency::eSandboxCurrency::EUR) return EUR;
        else if (currency==SandboxCurrency::eSandboxCurrency::GBP) return GBP;
        else if (currency==SandboxCurrency::eSandboxCurrency::HKD) return HKD;
        else if (currency==SandboxCurrency::eSandboxCurrency::CHF) return CHF;
        else if (currency==SandboxCurrency::eSandboxCurrency::JPY) return JPY;
        else if (currency==SandboxCurrency::eSandboxCurrency::CNY) return CNY;
        else if (currency==SandboxCurrency::eSandboxCurrency::TRY) return TRY;
        else 
        {
            throw std::runtime_error("Unknown CURRENCY value");
        }
    }

    QVector<CurrencyConfig> getCurrenciesConfigs( const QVector<SandboxCurrency::eSandboxCurrency> &v ) const
    {
        QVector<CurrencyConfig> res;

        auto b = v.begin();
        auto e = v.end();

        for(; b!=e; ++b)
        {
            res.push_back( getCurrenciesConfig(*b) );
        }

        return res;
    }

    QVector<CurrencyConfig> getCurrenciesConfigs( const QVector<QString> &v ) const
    {
        QVector<CurrencyConfig> res;

        auto b = v.begin();
        auto e = v.end();

        for(; b!=e; ++b)
        {
            res.push_back( getCurrenciesConfig(*b) );
        }

        return res;
    }

    QVector<CurrencyConfig> getCurrenciesConfigs( const QStringList &v ) const
    {
        return getCurrenciesConfigs(toVector(v));
    }

    QVector<CurrencyConfig> getCurrenciesConfigs( QString v ) const
    {
        v.replace(':', ";"); // replace *nix style list separator to windows style separator
        return getCurrenciesConfigs( v.split( ';', Qt::SkipEmptyParts ) );
    }

// Constructors

    CurrenciesConfig( const QSettings &settings )
    {
        load(settings);
    }

    CurrenciesConfig( const QString &settingsFile )
    {
        QSettings settings(settingsFile, QSettings::IniFormat);
        load(settings);
    }

}; // class CurrenciesConfig




//inline
//template<StreamType>
//print


} // namespace invest_openapi

