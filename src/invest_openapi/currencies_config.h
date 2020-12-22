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


class CurrenciesConfig;


class CurrencyConfig
{

    friend class CurrenciesConfig;

public:

    CurrencyConfig()
    : m_isValid(false)
    , m_value(0.0)
    {}

    inline bool fromQVariant( const QVariant qv )
    {
        if (qv.isValid() && !qv.isNull())
        {
            m_isValid = true;
            m_value   = qv.toDouble();
            return isValid();
        }
        else
        {
            m_isValid = false;
            return isValid();
        }
    }

    inline void fromSettings( const QSettings &settings, const QString &valueName )
    {
        if (fromQVariant(settings.value(valueName)))
        {
            QStringList nameSplitted = valueName.split( '.', Qt::SkipEmptyParts );
            if (nameSplitted.isEmpty())
            {
                throw std::runtime_error("Invalid currency property value taken");
            }

            QString currencyName = nameSplitted.back();
            m_currency = fromString( currencyName );
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

    SandboxCurrency::eSandboxCurrency getCurrency( ) const
    {
        return m_currency;
    }
    
    SandboxCurrency::eSandboxCurrency getType( ) const
    {
        return m_currency;
    }
    
    QString getName( ) const
    {
        return toString(m_currency);
    }


protected:

    static QString toString( SandboxCurrency::eSandboxCurrency c )
    {
        SandboxCurrency sbc;
        sbc.setValue(c);
        if (!sbc.isSet() || !sbc.isValid() || sbc.asJson().isEmpty())
        {
            throw std::runtime_error("Unknown CURRENCY value");
        }

        return sbc.asJson();
    }

    static SandboxCurrency::eSandboxCurrency fromString( const QString c )
    {
        SandboxCurrency sbc;
        sbc.fromJson(c);
        if (!sbc.isSet() || !sbc.isValid())
        {
            throw std::runtime_error("Unknown CURRENCY value");
        }

        return sbc.getValue();
    }


    bool     m_isValid;
    double   m_value;
    SandboxCurrency::eSandboxCurrency m_currency;


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
        RUB.fromSettings(settings, "sandbox.currency.RUB");
        USD.fromSettings(settings, "sandbox.currency.USD");
        EUR.fromSettings(settings, "sandbox.currency.EUR");
        GBP.fromSettings(settings, "sandbox.currency.GBP");
        HKD.fromSettings(settings, "sandbox.currency.HKD");
        CHF.fromSettings(settings, "sandbox.currency.CHF");
        JPY.fromSettings(settings, "sandbox.currency.JPY");
        CNY.fromSettings(settings, "sandbox.currency.CNY");
        TRY.fromSettings(settings, "sandbox.currency.TRY");
    }


public:

    CurrenciesConfig() {}

    CurrencyConfig getCurrencyConfig( const SandboxCurrency::eSandboxCurrency &currency ) const
    {
        switch (currency)
        {
            case SandboxCurrency::eSandboxCurrency::RUB: return RUB;
            case SandboxCurrency::eSandboxCurrency::USD: return USD;
            case SandboxCurrency::eSandboxCurrency::EUR: return EUR;
            case SandboxCurrency::eSandboxCurrency::GBP: return GBP;
            case SandboxCurrency::eSandboxCurrency::HKD: return HKD;
            case SandboxCurrency::eSandboxCurrency::CHF: return CHF;
            case SandboxCurrency::eSandboxCurrency::JPY: return JPY;
            case SandboxCurrency::eSandboxCurrency::CNY: return CNY;
            case SandboxCurrency::eSandboxCurrency::TRY: return TRY;
            default:
                break;
        }

        throw std::runtime_error("Unknown CURRENCY value");

        return RUB;
    }

    CurrencyConfig getCurrencyConfig( const QString &currencyName ) const
    {
        return getCurrencyConfig( CurrencyConfig::fromString( currencyName ) );
    }


    QVector<CurrencyConfig> getCurrenciesConfigs( const QVector<SandboxCurrency::eSandboxCurrency> &v ) const
    {
        QVector<CurrencyConfig> res;

        auto b = v.begin();
        auto e = v.end();

        for(; b!=e; ++b)
        {
            CurrencyConfig cfg = getCurrencyConfig(*b);
            if (cfg.isValid())
                res.push_back( cfg );
        }

        return res;
    }

    QVector<CurrencyConfig> getCurrencyConfigs( const QVector<QString> &v ) const
    {
        QVector<CurrencyConfig> res;

        auto b = v.begin();
        auto e = v.end();

        for(; b!=e; ++b)
        {
            CurrencyConfig cfg = getCurrencyConfig(*b);
            if (cfg.isValid())
                res.push_back( cfg );
        }

        return res;
    }

    QVector<CurrencyConfig> getCurrencyConfigs( const QStringList &v ) const
    {
        return getCurrencyConfigs(v.toVector());
    }

    QVector<CurrencyConfig> getCurrencyConfigs( QString v ) const
    {
        return getCurrencyConfigs( listStringSplit(v) );
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

