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
#include <sstream>

#include "invest_openapi.h"
#include "qt_time_helpers.h"
#include "marty_decimal.h"

#include "../cpp/cpp.h"

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
// https://en.cppreference.com/w/cpp/language/adl
// https://stackoverflow.com/questions/4603886/c-operator-lookup-rules-koenig-lookup/4604017

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
std::ostream& operator<<( std::ostream& os, const QString &str )
{
    if (str.isEmpty())
        return os;

    // os << str.toStdString();
    auto ba = str.toLocal8Bit();
    os << std::string(ba.begin(),ba.end());

    return os;
}

//------------------------------
inline
std::ostream& operator<<( std::ostream& os, const QDateTime &v )
{
    if (v.isNull())
    {
        os<<"<NULL>";
        return os;
    }

    if (v.isValid())
    {
        os<<"<INVALID>";
        return os;
    }

    os << qt_helpers::dateTimeToDbString(v); // .toStdString();

    return os;
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
//namespace invest_openapi
namespace OpenAPI
{



//----------------------------------------------------------------------------
#define OPENAPI_IMPLEMENT_OSTREAM_INSERTER( OpenApiType )                             \
            inline std::ostream& operator<<( std::ostream& os, const OpenApiType &v ) \
            {                                                                         \
                os << v.asJson();                                                     \
                return os;                                                            \
            }

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
OPENAPI_IMPLEMENT_OSTREAM_INSERTER( OperationStatus )
OPENAPI_IMPLEMENT_OSTREAM_INSERTER( Currency )
OPENAPI_IMPLEMENT_OSTREAM_INSERTER( InstrumentType )
OPENAPI_IMPLEMENT_OSTREAM_INSERTER( OperationTypeWithCommission )
//OPENAPI_IMPLEMENT_OSTREAM_INSERTER(  )
//OPENAPI_IMPLEMENT_OSTREAM_INSERTER(  )
//OPENAPI_IMPLEMENT_OSTREAM_INSERTER(  )

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
template< typename OpenApiType > inline
bool isSetAndValid( const OpenApiType &v )
{
    throw std::runtime_error("OpenAPI::isSetAndValid not implemented for this type");
}

//------------------------------
template< > inline
bool isSetAndValid<MoneyAmount>( const MoneyAmount &v )
{
    return v.isSet() && v.isValid();
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
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
template < typename StreamType, typename ValueType > inline 
StreamType& insertAlignedToRight( StreamType &os, const ValueType &v, std::size_t width )
{
    // Чёт лень с манипуляторами возиться, поэтому запарился по выравниванию сам

    std::ostringstream ss;
    ss << v;
    os << cpp::expandAtFront( ss.str(), width );

    return os;
}

//----------------------------------------------------------------------------
template < typename StreamType, typename ValueType > inline 
StreamType& insertAlignedToLeft( StreamType &os, const ValueType &v, std::size_t width )
{
    // Чёт лень с манипуляторами возиться, поэтому запарился по выравниванию сам

    std::ostringstream ss;
    ss << v;
    os << cpp::expandAtBack( ss.str(), width );

    return os;
}

//----------------------------------------------------------------------------
template < typename StreamType, typename ValueType1, typename ValueType2 > inline 
StreamType& insertAlignedToRight( StreamType &os, const ValueType1 &v1, const ValueType2 &v2, std::size_t width )
{
    // Чёт лень с манипуляторами возиться, поэтому запарился по выравниванию сам

    std::ostringstream ss;
    ss << v1 << " " << v2;
    os << cpp::expandAtFront( ss.str(), width );

    return os;
}

//----------------------------------------------------------------------------
template < typename StreamType, typename ValueType1, typename ValueType2 > inline 
StreamType& insertAlignedToLeft( StreamType &os, const ValueType1 &v1, const ValueType2 &v2, std::size_t width )
{
    // Чёт лень с манипуляторами возиться, поэтому запарился по выравниванию сам

    std::ostringstream ss;
    ss << v1 << " " << v2;
    os << cpp::expandAtBack( ss.str(), width );

    return os;
}

//----------------------------------------------------------------------------
inline std::ostream& operator<<( std::ostream& os, const OperationTrade &v )
{
    // Чёт лень с манипуляторами возиться, поэтому запарился по выравниванию сам

    // os << "ID: ";
    insertAlignedToRight( os, v.getTradeId() , 12 ) << ", ";

    os << "at ";
    insertAlignedToRight( os, v.getDate()    , 23 ) << ", ";

    insertAlignedToRight( os, v.getQuantity(), 5  ) << " x ";
    insertAlignedToRight( os, v.getPrice()   , 8  ) << " = ";
    
    insertAlignedToRight( os, v.getQuantity() * v.getPrice()   , 9  );
    //os << ( v.getQuantity() * v.getPrice() );

    return os;
}

//----------------------------------------------------------------------------
inline std::ostream& operator<<( std::ostream& os, const Operation &v )
{
    // Чёт лень с манипуляторами возиться, поэтому запарился по выравниванию сам

    using std::endl;

    os << "Operation : ";
    insertAlignedToLeft( os, v.getId()               , 12 ) << " - ";
    os << v.getOperationType() << " - " << v.getStatus() << endl;

    //insertAlignedToRight( os, v.getStatus()           ,  8 ) << endl;

    os << "Instrument: ";
    insertAlignedToRight( os, v.getFigi()             , 12 ) << " / " << v.getInstrumentType() << endl;
    //insertAlignedToRight( os, v.getInstrumentType()   , 6  ) << ", ";

    //insertAlignedToRight( os, v.getDate()             , 24 ) << " - FIGI: ";
    //os << v.getOperationType() << endl;
    //insertAlignedToRight( os, v.getOperationType()    , 12 ) << endl;

    os << "Date&time : " << v.getDate() << endl;

    os << "Payment   : ";
    // insertAlignedToRight( os, v.getPayment()          , 10 ) << " ";
    // insertAlignedToRight( os, v.getCurrency()         ,  4 ) << ", ";
    insertAlignedToRight( os, v.getPayment(), v.getCurrency() , 14 ) << endl ; // ", ";

    os << "Commission: ";
    insertAlignedToRight( os, v.getCommission()       , 14 ) << endl;

    os << "Quantity  : ";
    insertAlignedToRight( os, v.getQuantity()         ,  6 ) << endl; // ", ";

    os << "Executed  : ";
    insertAlignedToRight( os, v.getQuantityExecuted() ,  6 ) << " x ";
    insertAlignedToRight( os, v.getPrice()            ,  8 ) << endl;

    if (v.isIsMarginCall())
    {
        os << "Nike Margin called me" << endl;
    }   

    QList<OperationTrade> trades = v.getTrades();

    if (trades.size()<=0)
        return os;

    
    os << "Trades    : "; // << endl;
    bool bFirst = true;
    for( const auto &trade : trades ) 
    {
        if (!bFirst)
            os << "            ";
        else
            bFirst = false;

        os << trade << endl;
    }

    return os;

}



/*

            cout << "Price             : " << op.getPrice()            << endl;
            cout << "Nike called       : " << (op.isIsMarginCall()?"yes":"no") << endl;



makeExpandString( const std::basic_string< CharT, Traits, Allocator > &str, std::size_t expandToSize, CharT ch = (CharT)' ' )
expandAtFront( const std::basic_string< CharT, Traits, Allocator > &str, std::size_t expandToSize, CharT ch = (CharT)' ')
expandAtBack( const std::basic_string< CharT, Traits, Allocator > &str, std::size_t expandToSize, CharT ch = (CharT)' ')

*/



//----------------------------------------------------------------------------


//} // namespace invest_openapi
} // namespace OpenAPI

