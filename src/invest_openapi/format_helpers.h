#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>


#include "../cpp/cpp.h"
#include "marty_decimal.h"


namespace invest_openapi
{


struct FieldFormat
{
    std::size_t leftSpace        ;
    std::size_t rightSpace       ;
    int         fieldWidth       ; //!< <0 - Elipsis allowed for this field
    int         alignment        ; //!< left - <0, right - >0, center - ==0

    std::size_t dotAlignment = 0 ; //!< For Decimal's only

    QString     id               = QString(); //!< Field ID - to make fields order more dynamic

    int         captionAlignment = 0 ;
    QString     caption          = QString(); //!< Not used for generic fields, only for caption


    #if defined(INVEST_OPENAPI_TEXT_TERMINAL_ENABLED_COLORS)

        umba::term::colors::SgrColor   color = 0; // black used as no color value

    #endif

    std::size_t getSummaryWidth() const
    {
        std::size_t w = (std::size_t)( fieldWidth>0 ? fieldWidth : -fieldWidth );
        return leftSpace + rightSpace + w;
    }

    bool hasCustomColor() const
    {
        #if defined(INVEST_OPENAPI_TEXT_TERMINAL_ENABLED_COLORS)

            return color!=0;

        #else

            return false;

        #endif
    }



}; // struct FieldFormat




//----------------------------------------------------------------------------
template< typename T > inline
std::string format_field( std::size_t leftSpace, std::size_t rightSpace, int fieldWidthEx, int align, const T &val )
{
    std::ostringstream oss;
    oss << val;

    std::string strRes = oss.str();

    bool bAddEllipsis = false;

    if (fieldWidthEx<0)
    {
        fieldWidthEx = -fieldWidthEx;
        bAddEllipsis = true;
    }

    std::size_t fieldWidth = (std::size_t)fieldWidthEx;


    if (strRes.size()>fieldWidth && bAddEllipsis)
    {
        strRes.erase( fieldWidth, strRes.npos ); // Укоротили

        if (strRes.size()>6) // Добавлям элипсис. Иначе - просто отрезаем лишнее, потому как если окончание строки заменить на элипсис, то нихрена не понятно будет
        {
            strRes.erase( strRes.size()-3, strRes.npos ); // Освободили место под элипсис
            strRes = marty_cpp::expandAtBack( strRes, fieldWidth, '.' );
        }
    }


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
    strRes = marty_cpp::expandAtFront( strRes, strRes.size()+leftSpace  );

    return strRes;
}

//----------------------------------------------------------------------------
inline
int format_field_decimalAdjustPrecision( const marty::Decimal &d, int precision )
{
    if (precision<0)
    {
        // Exact Decimal number precision will be used
        precision = d.precision();
    }
    else if (precision>0)
    {
        // Exact taken precision will be used
    }
    else // precision==0
    {
        // Output stream precision will be used
        precision = d.getOutputPrecision();
        if (precision<=0)
            precision = d.precision();
    }

    return precision;

}

//----------------------------------------------------------------------------
inline
std::string format_field( std::size_t leftSpace, std::size_t rightSpace, int fieldWidth, int align, marty::Decimal d, int precision = -1 )
{
    precision = format_field_decimalAdjustPrecision(d,precision);

    // Числа никогда не обрезаем
    if (fieldWidth<0)
        fieldWidth = -fieldWidth;

    d.round( precision, marty::Decimal::RoundingMethod::roundMath );

    return format_field<std::string>( leftSpace, rightSpace, fieldWidth, align, d.toString( precision ) );

}

//----------------------------------------------------------------------------
inline
std::string format_field( std::size_t leftSpace, std::size_t rightSpace, int fieldWidth, int align
                        , marty::Decimal d, int precision
                        , std::size_t dotAlign // Выравнивание по десятичной точке
                        )
{
    precision = format_field_decimalAdjustPrecision(d,precision);

    if (precision<0)
        precision = marty::Decimal::getOutputPrecision();

    if (fieldWidth<0)
        fieldWidth = -fieldWidth;

    d.precisionExpandTo(precision+1);
    d.round( precision, marty::Decimal::RoundingMethod::roundMath );

    std::string numberStr = d.toString( precision );

    if (dotAlign>(std::size_t)precision)
    {
        std::size_t extraSpace = dotAlign - (std::size_t)precision;
        numberStr = marty_cpp::expandAtBack ( numberStr, numberStr.size()+extraSpace );
    }

    return format_field<std::string>( leftSpace, rightSpace, fieldWidth, align, numberStr );

}

//----------------------------------------------------------------------------
inline
std::string format_field( std::size_t leftSpace, std::size_t rightSpace, int fieldWidth, int align
                        , marty::Decimal d
                        , const marty::Decimal &numberWithRequiredPrecision // priceIncrement is a good candidate
                        , std::size_t dotAlign // Выравнивание по десятичной точке
                        )
{
    return format_field( leftSpace, rightSpace, fieldWidth, align
                        , d, numberWithRequiredPrecision.precision()
                        , dotAlign
                        );
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
template< typename T > inline
std::string format_field( const FieldFormat &fmt, const T &val )
{
    return format_field<T>( fmt.leftSpace, fmt.rightSpace, fmt.fieldWidth, fmt.alignment, val );
}

//----------------------------------------------------------------------------
inline
std::string format_field( const FieldFormat &fmt, marty::Decimal d, int precision = -1 )
{
    if (fmt.dotAlignment)
        return format_field( fmt.leftSpace, fmt.rightSpace, fmt.fieldWidth, fmt.alignment, d, precision, fmt.dotAlignment );
    else
        return format_field( fmt.leftSpace, fmt.rightSpace, fmt.fieldWidth, fmt.alignment, d, precision );
}

//----------------------------------------------------------------------------
inline
std::string format_field( const FieldFormat &fmt
                        , marty::Decimal d
                        , const marty::Decimal &numberWithRequiredPrecision // priceIncrement is a good candidate
                        )
{
    return format_field( fmt, d, numberWithRequiredPrecision.precision() );
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
std::string format_field_caption( const FieldFormat &fmt )
{
    QString caption = fmt.caption;

    if (caption.isEmpty())
        caption = fmt.id;

    if (caption=="_")
        caption = " ";

    int fieldWidth = fmt.fieldWidth;
    if (fieldWidth>0)
        fieldWidth = -fieldWidth; // Allow elipsis for captions

    return format_field<std::string>( fmt.leftSpace, fmt.rightSpace, fieldWidth, fmt.captionAlignment, caption.toStdString() );
}

//----------------------------------------------------------------------------






//----------------------------------------------------------------------------




} // namespace invest_openapi

//----------------------------------------------------------------------------
