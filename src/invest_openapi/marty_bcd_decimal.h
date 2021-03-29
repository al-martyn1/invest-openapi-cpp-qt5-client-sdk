#pragma once


#define MARTY_DECIMAL_H__079F0131_B01B_44ED_BF0F_8DFECAB67FD7__

#include <string>
#include <cstdint>
#include <utility>
#include <limits>
#include <iostream>
#include <exception>
#include <stdexcept>



#include "marty_bcd.h"


//----------------------------------------------------------------------------
namespace marty
{

class Decimal
{

public:

    //------------------------------
    enum class RoundingMethod
    {
        // https://en.wikipedia.org/wiki/Rounding

        roundingInvalid                               ,


        // Directed rounding to an integer methods

        roundDown                                     , // roundFloor, roundTowardNegInf
        roundFloor        = roundDown                 ,
        roundTowardNegInf = roundDown                 , // round towards negative infinity
        
        roundUp                                       , // roundCeil, roundTowardsPosInf
        roundCeil          = roundUp                  ,
        roundTowardsPosInf = roundUp                  , // round towards positive infinity

        roundTowardsZero                              , // roundAwayFromInf, roundTrunc
        roundAwayFromInf  = roundTowardsZero          ,
        roundTrunc        = roundTowardsZero          ,

        roundTowardsInf                               , // roundAwayFromZero
        roundAwayFromZero = roundTowardsInf           ,


        // Rounding to the nearest integer

        roundHalfUp                                   , // roundHalfTowardsPositiveInf
        roundHalfTowardsPositiveInf = roundHalfUp     ,

        roundHalfDown                                 , // roundHalfTowardsNegativeInf
        roundHalfTowardsNegativeInf = roundHalfDown   ,

        roundHalfTowardsZero                          , // roundHalfAwayFromInf
        roundHalfAwayFromInf  = roundHalfTowardsZero  ,

        roundHalfTowardsInf                           , // roundHalfAwayFromZero, roundMath
        roundHalfAwayFromZero = roundHalfTowardsInf   ,
        roundMath             = roundHalfTowardsInf   ,

        roundHalfToEven                               , // roundBankers, roundBanking, roundConvergent, roundStatistician, roundStatistic, roundDutch, roundGaussian
        roundBankers      = roundHalfToEven           ,
        roundBanking      = roundHalfToEven           ,
        roundConvergent   = roundHalfToEven           ,
        roundStatistician = roundHalfToEven           ,
        roundStatistic    = roundHalfToEven           ,
        roundDutch        = roundHalfToEven           ,
        roundGaussian     = roundHalfToEven           ,

        roundHalfToOdd

    };

    //------------------------------



    //------------------------------
    static Decimal fromString( const char        *pStrDecimal );
    static Decimal fromString( const std::string &strDecimal  );

#if 0

    std::string   toString( int precision = -1 ) const;

    int           toInt() const                           { return (int)(m_num/m_denum.denum()); }
    unsigned      toUnsigned() const                      { return (unsigned)(m_num/m_denum.denum()); }
    std::int64_t  toInt64() const                         { return (m_num/m_denum.denum()); }
    std::uint64_t toUnsigned64() const                    { return (std::uint64_t)(m_num/m_denum.denum()); }
    float         toFloat() const                         { return (float)((double)m_num/(double)m_denum.denum()); }
    double        toDouble() const                        { return ((double)m_num/(double)m_denum.denum()); }
    //------------------------------



    //------------------------------
    bool checkIsExact( const std::string &strDecimal  ) const { return toString( -1 )==strDecimal; }
    bool checkIsExact( const char        *pStrDecimal ) const { return checkIsExact(std::string(pStrDecimal)); }

    //------------------------------



    //------------------------------
    Decimal()                      {}
    Decimal( const Decimal &d ) {}

    //------------------------------
    // 
    Decimal( int            v, int precision = 0 ) {  /* m_num *= m_denum.denum(); */  }
    Decimal( unsigned       v, int precision = 0 ) {  /* m_num *= m_denum.denum(); */  }
    Decimal( std::int64_t   v, int precision = 0 ) {  /* m_num *= m_denum.denum(); */  }
    Decimal( std::uint64_t  v, int precision = 0 ) {  /* m_num *= m_denum.denum(); */  }

    //------------------------------
    //
    Decimal( float          f, int precision = 0 ) { }
    Decimal( double         f, int precision = 0 ) { }
    //------------------------------

    Decimal( const std::string &strDecimal  ) : m_num(0), m_denum(0) { *this = fromString(strDecimal ); }
    Decimal( const char        *pStrDecimal ) : m_num(0), m_denum(0) { *this = fromString(pStrDecimal); }

    //------------------------------



    //------------------------------
    void swap( Decimal &d2 );
    Decimal& operator=( Decimal d2 );
    int compare( Decimal d2 ) const;

    //------------------------------



    //------------------------------
    // операторы преобразования типа 
    explicit operator int() const           { return toInt(); }
    explicit operator unsigned() const      { return toUnsigned(); }
    explicit operator std::int64_t() const  { return toInt64(); }
    explicit operator std::uint64_t() const { return toUnsigned64(); }
    explicit operator float() const         { return toFloat(); }
    explicit operator double() const        { return toDouble(); }

    //------------------------------
    Decimal operator + ( Decimal d2 ) const;
    Decimal operator - ( Decimal d2 ) const;
    Decimal operator - ( ) const;
    Decimal operator * ( Decimal d2 ) const;
    Decimal divide( Decimal devider, precision_t resultPrecision ) const;
    Decimal operator / ( Decimal d2 ) const;
    Decimal operator % ( Decimal d2 ) const;
    Decimal& operator += ( Decimal d2 );
    Decimal& operator -= ( Decimal d2 );
    Decimal& operator *= ( Decimal d2 );
    Decimal& operator /= ( Decimal d2 );
    Decimal& operator %= ( Decimal d2 );

    //------------------------------
    #define MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE( integralType )         \
                                                                                                    \
                Decimal operator +   ( integralType i ) const { return operator+ ( Decimal(i) ); }  \
                Decimal operator -   ( integralType i ) const { return operator- ( Decimal(i) ); }  \
                Decimal operator *   ( integralType i ) const { return operator* ( Decimal(i) ); }  \
                Decimal operator /   ( integralType i ) const { return operator/ ( Decimal(i) ); }  \
                Decimal operator %   ( integralType i ) const { return operator% ( Decimal(i) ); }  \
                                                                                                    \
                Decimal& operator += ( integralType i )       { return operator+=( Decimal(i) ); }  \
                Decimal& operator -= ( integralType i )       { return operator-=( Decimal(i) ); }  \
                Decimal& operator *= ( integralType i )       { return operator*=( Decimal(i) ); }  \
                Decimal& operator /= ( integralType i )       { return operator/=( Decimal(i) ); }  \
                Decimal& operator %= ( integralType i )       { return operator%=( Decimal(i) ); }


    MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE( int           )
    MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE( unsigned      )
    MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE( std::int64_t  )
    MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE( std::uint64_t )
    MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE( float         )
    MARTY_DECIMAL_IMPLEMENT_ARIPHMETICT_OVERLOADS_FOR_INTEGRAL_TYPE( double        )

    //----------------------------------------------------------------------------

    MARTY_IMPLEMENT_RELATIONAL_OPERATORS_BY_COMPARE( typeT2 )


public:

    //------------------------------
    Decimal minimizePrecision() const;
    Decimal& minimizePrecisionInplace();
    Decimal expantPrecisionTo( precision_t p ) const;
    Decimal shrinkPrecisionTo( precision_t p ) const;
    Decimal fitPrecisionTo   ( precision_t p ) const;

    //------------------------------
    Decimal& negate();
    Decimal& invert() { return negate(); }

    int     sign() const;
    int     sgn () const { return sign(); }
    Decimal abs () const;
    Decimal mod (  Decimal d2 ) const;
    Decimal neg () const;
    Decimal inv () const { return neg(); }

    //------------------------------
    Decimal getPercentOf( Decimal d ) const;
    Decimal getPermilleOf( Decimal d ) const;

    Decimal rounded( precision_t precision, RoundingMethod roundingMethod ) const;

    //------------------------------

    static precision_t getOutputPrecision()                   { return m_outputPrecision; }
    static void        setOutputPrecision( precision_t p )    { m_outputPrecision = p; }
    static void        setOutputPrecisionToStreamPrecision( ) { setOutputPrecision(0); }
    static void        setOutputPrecisionToAuto( )            { setOutputPrecision((precision_t)-1); }

    //------------------------------
    //------------------------------
    int compare( num_t n ) const
    {
        if (m_num<n) return -1;
        if (m_num>n) return  1;
        return 0;
    }

    //------------------------------
    //! Возвращает true, если обрезание/удлиннение прошло предельно точно и ничего лишнего не было задето, и все жизненно важные органы остались на месте
    bool precisionExpandTo( precision_t p );
    bool precisionShrinkTo( precision_t p );
    bool precisionFitTo( precision_t p );

    //------------------------------
    static
    bool isDigitEven( unum_t d ) // Чётно?
    {
        switch(d)
        {
            case 0: case 2: case 4: case 6: case 8:
            return true;
        }
        
        return false;
    }
    
    //------------------------------
    static
    bool isDigitOdd( unum_t d ) // Нечётно?
    {
        switch(d)
        {
            case 1: case 3: case 5: case 7: case 9:
            return true;
        }
        
        return false;
    }

    //------------------------------
    Decimal& minimizePrecisionImpl();
    unum_t getLowestDecimalDigit() const;
    void replaceLowestDecimalDigit( unum_t d );

    Decimal makeMinimalPrecisionImplHelper( num_t newNum ) const;
    Decimal makeMinimalPrecisionOne() const;
    Decimal makeMinimalPrecisionTwo() const;
    Decimal makeMinimalPrecisionFive() const;

    //------------------------------


#endif

protected:


    Decimal( int sign, int precision, bcd::raw_bcd_number_t number )
    : m_sign(sign), m_precision(precision), m_number(number)
    {}

    int                     m_sign;
    int                     m_precision;
    bcd::raw_bcd_number_t   m_number;
    //------------------------------
    // Global affecting options

    // if == -1  - Exact Decimal number precision will be used
    // if ==  0  - Output stream precision will be used
    // if >   0  - Exact precision will be used
    inline static precision_t         m_outputPrecision = (precision_t)-1; 


}; // class Decimal



//----------------------------------------------------------------------------

#include "marty_bcd_decimal_impl.h"

//----------------------------------------------------------------------------



} // namespace marty


//----------------------------------------------------------------------------

