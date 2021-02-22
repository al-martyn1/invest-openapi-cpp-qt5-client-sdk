/*! \file
    \brief 
 */

#pragma once

#include <QObject>
#include <QDebug>
#include <QtDebug>
#include <QTest>
#include <QTimeZone>
#include <QDate>
#include <QTime>
#include <QDateTime>

#include <exception>
#include <stdexcept>

#include "invest_openapi.h"
#include "qt_time_helpers.h"
#include "marty_decimal.h"


// https://en.cppreference.com/w/cpp/language/adl
// https://stackoverflow.com/questions/4603886/c-operator-lookup-rules-koenig-lookup/4604017

inline
std::ostream& operator<<( std::ostream& os, const QString &str )
{
    // os << str.toStdString();
    auto ba = str.toLocal8Bit();
    os << std::string(ba.begin(),ba.end());

    return os;
}

inline
std::ostream& operator<<( std::ostream& os, const QDateTime &v )
{
    os << qt_helpers::dateTimeToDbString(v); // .toStdString();
    return os;
}



//----------------------------------------------------------------------------
//namespace invest_openapi
namespace OpenAPI
{


#define OPENAPI_IMPLEMENT_OSTREAM_INSERTER( OpenApiType )                             \
            inline std::ostream& operator<<( std::ostream& os, const OpenApiType &v ) \
            {                                                                         \
                os << v.asJson();                                                     \
                return os;                                                            \
            }

OPENAPI_IMPLEMENT_OSTREAM_INSERTER( OperationStatus )
OPENAPI_IMPLEMENT_OSTREAM_INSERTER( Currency )
OPENAPI_IMPLEMENT_OSTREAM_INSERTER( InstrumentType )
OPENAPI_IMPLEMENT_OSTREAM_INSERTER( OperationTypeWithCommission )
//OPENAPI_IMPLEMENT_OSTREAM_INSERTER(  )
//OPENAPI_IMPLEMENT_OSTREAM_INSERTER(  )
//OPENAPI_IMPLEMENT_OSTREAM_INSERTER(  )
//OPENAPI_IMPLEMENT_OSTREAM_INSERTER(  )
//OPENAPI_IMPLEMENT_OSTREAM_INSERTER(  )
//OPENAPI_IMPLEMENT_OSTREAM_INSERTER(  )


// OPENAPI_IMPLEMENT_OSTREAM_INSERTER( MoneyAmount )
/*
    Currency getCurrency() const;
    marty::Decimal getValue() const;

    Currency getCurrency() const;
    void setCurrency(const Currency &currency);
    bool is_currency_Set() const;
    bool is_currency_Valid() const;

    marty::Decimal getValue() const;
    void setValue(const marty::Decimal &value);
    bool is_value_Set() const;
    bool is_value_Valid() const;

*/

template< typename OpenApiType > inline
bool isSetAndValid( const OpenApiType &v )
{
    throw std::runtime_error("OpenAPI::isSetAndValid not implemented for this type");
}

template< > inline
bool isSetAndValid<MoneyAmount>( const MoneyAmount &v )
{
    return v.isSet() && v.isValid();
}


inline std::ostream& operator<<( std::ostream& os, const MoneyAmount &v )
{
    if ( isSetAndValid(v) && v.is_currency_Set() && v.is_currency_Valid() && v.is_value_Set() && v.is_value_Valid() )
    {
        os << v.getValue() << " " << v.getCurrency();
        return os;
    }

    os << "0?";
    return os;
}





//----------------------------------------------------------------------------


//} // namespace invest_openapi
} // namespace OpenAPI

