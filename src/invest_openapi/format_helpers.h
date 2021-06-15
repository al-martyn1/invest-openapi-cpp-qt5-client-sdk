#pragma once

#include <iostream>
#include <sstream>


#include "../cpp/cpp.h"
#include "marty_decimal.h"


namespace invest_openapi
{


// Alignment left    - <0
//           right   - >0
//           center  - ==0


//----------------------------------------------------------------------------
template< typename T > inline
std::string format_field( std::size_t leftSpace, std::size_t rightSpace, std::size_t fieldWidth, int align, T val )
{
    std::ostringstream oss;
    oss << val;

    std::string strRes = oss.str();

    if (align<0) // to left
    {
        strRes = marty_cpp::expandAtBack( strRes, fieldWidth );
    }
    else if (align>0) // to right
    {
        strRes = marty_cpp::expandAtFront( strRes, fieldWidth );
    }
    else // if (align==0)
    {
        if ( strRes.size() < fieldWidth )
        {
            // need to add extra space both at left and right
            auto extraTotal = fieldWidth - strRes.size();
            auto extra2     = extraTotal/2;
            strRes = marty_cpp::expandAtBack ( strRes, strRes.size()+extra2 );
            strRes = marty_cpp::expandAtFront( strRes, fieldWidth );
        }
    }

    strRes = marty_cpp::expandAtBack ( strRes, strRes.size()+rightSpace );
    strRes = marty_cpp::expandAtFront( strRes, strRes.size()+leftSpace+rightSpace );

    return strRes;
}

//----------------------------------------------------------------------------
inline
std::string format_field( std::size_t leftSpace, std::size_t rightSpace, std::size_t fieldWidth, int align, marty::Decimal d, int precision = -1 )
{
    if (precision<0)
        precision = marty::Decimal::getOutputPrecision();

    return format_field<std::string>( leftSpace, rightSpace, fieldWidth, align, d.toString( precision ) );

}

//----------------------------------------------------------------------------




/*

template< class CharT, class Traits = std::char_traits<CharT>, class Allocator = std::allocator<CharT> >
inline std::basic_string< CharT, Traits, Allocator >
makeExpandString( const std::basic_string< CharT, Traits, Allocator > &str, std::size_t expandToSize, CharT ch = (CharT)' ' )


template< class CharT, class Traits = std::char_traits<CharT>, class Allocator = std::allocator<CharT> >
inline std::basic_string< CharT, Traits, Allocator >
expandAtFront( const std::basic_string< CharT, Traits, Allocator > &str, std::size_t expandToSize, CharT ch = (CharT)' ')


template< class CharT, class Traits = std::char_traits<CharT>, class Allocator = std::allocator<CharT> >
inline std::basic_string< CharT, Traits, Allocator >
expandAtBack( const std::basic_string< CharT, Traits, Allocator > &str, std::size_t expandToSize, CharT ch = (CharT)' ')


*/







//----------------------------------------------------------------------------




} // namespace invest_openapi

//----------------------------------------------------------------------------
