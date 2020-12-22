/*! \file
    \brief 
 */

//----------------------------------------------------------------------------
#pragma once

#include <QString>
#include <QUrl>

#include <exception>
#include <stdexcept>

#include "models.h"
// #include "utility.h"

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
namespace invest_openapi
{




//----------------------------------------------------------------------------
template<typename MonetaryType>
MonetaryType toMonetary( const QVariant &s );

template<typename MonetaryType>
MonetaryType toMonetary( const QString &s );

template<typename MonetaryType>
QVariant monetaryToVariant( const MonetaryType &m );

template<typename MonetaryType>
QString monetaryToString( const MonetaryType &m );

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
// Like MarketInstrument, but more C++ ready
template<typename MonetaryType>
struct Instrument
{
    typedef             Currency::eCurrency               currency_type  ;
    typedef             InstrumentType::eInstrumentType   instrument_type;

    bool                isValid          ;

    QString             figi             ;
    QString             ticker           ;
    QString             isin             ;
    MonetaryType        minPriceIncrement;
    qint32              lotSize          ;
    qint32              minQuantity      ;
    currency_type       currency         ;
    QString             name             ;
    instrument_type     instrumentType   ;


    Instrument( const MarketInstrument &i )
    {
        assign(i);
    }

    Instrument& operator=( const MarketInstrument &i )
    {
        assign(i);
        return *this;
    }

    operator MarketInstrument() const
    {
        MarketInstrument i;
        if (isValid)
            return i;

        i.setFigi(figi);
        i.setTicker(ticker);
        i.setIsin(isin);
        i.setMinPriceIncrement(toMonetary<double>(monetaryToString<MonetaryType>(minPriceIncrement)));
        i.setLot(lotSize);
        i.setMinQuantity(minQuantity);
        i.setCurrency(toCurrency(currency));
        i.setName(name);
        i.setType(toInstrumentType(instrumentType));
    }

protected:

    void assign( const MarketInstrument &i )
    {
        isValid = true;

        if (!i.is_figi_Set())
        {
            isValid = false; 
            throw std::runtime_error("Instrument::assign - FIGI - don't allowed default");
        }
        else if (!i.is_figi_Valid())
        {
            isValid = false; 
            throw std::runtime_error("Instrument::assign - FIGI - invalid");
        }
        else
        {
            figi = i.getFigi();
        }

        if (!i.is_ticker_Set() || !i.is_ticker_Valid())
        {
            isValid = false;
            throw std::runtime_error("Instrument::assign - TICKER");
        }
        else 
        {
            ticker = i.getTicker();
        }

        if (!i.is_isin_Set() || !i.is_isin_Valid())                                { isValid = false; throw std::runtime_error("Instrument::assign - ISIN"); }
        else isin = i.getIsin();

        if (!i.is_min_price_increment_Set() || !i.is_min_price_increment_Valid())
        {
            isValid = false;
            throw std::runtime_error("Instrument::assign - PRICE INCREMENT");
        }
        else minPriceIncrement = toMonetary<MonetaryType>( QVariant(i.getMinPriceIncrement()));

        if (!i.is_lot_Set() || !i.is_lot_Valid())                                  { isValid = false; throw std::runtime_error("Instrument::assign - LOT"); }
        else lotSize = i.getLot();

        if (!i.is_min_quantity_Set() || !i.is_min_quantity_Valid())
        {
            //isValid = false;
            //throw std::runtime_error("Instrument::assign - QUANTITY");
            minQuantity = 1;
        }
        else 
        {
            minQuantity = i.getMinQuantity();
        }

        if (!i.is_currency_Set() || !i.is_currency_Valid())                        { isValid = false; throw std::runtime_error("Instrument::assign - CURRENCY"); }
        else currency = i.getCurrency().getValue();

        if (!i.is_name_Set() || !i.is_name_Valid())                                { isValid = false; throw std::runtime_error("Instrument::assign - NAME"); }
        else name = i.getName();

        if (!i.is_type_Set() || !i.is_type_Valid())                                { isValid = false; throw std::runtime_error("Instrument::assign - TYPE"); }
        else instrumentType = i.getType().getValue();
    }


}; // struct Instrument





//----------------------------------------------------------------------------


} // namespace invest_openapi

