#pragma once

#define MARTY_DECIMAL_H__F7FEF215_956E_4063_8AD1_1F66F6E17EB7__

#include <string>
#include <cstdint>
#include <utility>
#include <limits>
#include <iostream>
#include <exception>
#include <stdexcept>


//----------------------------------------------------------------------------
namespace marty
{


// Hm-m-m
// МСБ-Лизинг 002P выпуск 2
// Ticker: RU000A100DC4
// Price increment: 0.083336


//----------------------------------------------------------------------------
// 4 294 967 295 uint32_t max
// 1 000 000 000 - must be enough for money or use more wide type in typedef below

typedef std::uint32_t  DecimalDenumeratorRawType;
typedef std::int32_t   DecimalDenumeratorSignedRawType;

//----------------------------------------------------------------------------
//  9 223 372 036 854 775 808
//  1 000 000 000.000 000  000 - 10**9 -  миллиард - бюджет страны верстать не выйдет с точностью 9 нулей после точки
//  1 000 000 000 000 00.0 000 - 10**15 - 100 трлн с точностью до сотых копейки - для домашней бухгалтерии норм

typedef std::int64_t    DecimalNumeratorRawType;
typedef std::uint64_t   DecimalNumeratorUnsignedRawType;



//----------------------------------------------------------------------------
class DecimalDenumerator;
class Decimal;

class DecimalPrecision
{
    friend class DecimalDenumerator;
    friend class Decimal;

    std::uint32_t m_precision;

    // default constructor disabled
    DecimalPrecision( );
    //DecimalPrecision& operator=( const DecimalPrecision &dp );

public:

    typedef std::uint32_t                    precision_t;
    typedef DecimalDenumeratorRawType        denum_t;
    typedef DecimalDenumeratorSignedRawType  sdenum_t;

    static denum_t denum( precision_t pr )
    {
        if ( maxPrecision<denum_t>() < pr )
            throw std::runtime_error("DecimalPrecision::denum - taken precision to big");

        return (denum_t)(getPowers10Table()[pr]);

        /*
        denum_t  denum = 1;
        for(precision_t i=0; i!=pr; ++i)
        {
            denum_t next = denum * 10u;
            if (next<denum)
                throw std::runtime_error("DecimalPrecision precision to big");
            denum = next;
        }

        return denum;
        */
    }

    DecimalPrecision( std::uint32_t pr ) : m_precision(pr) {}
    DecimalPrecision( const DecimalPrecision &pr ) : m_precision(pr.m_precision) {}

    denum_t denum( ) const         { return denum(m_precision);  } // for compatibility
    denum_t denumerator( ) const   { return denum(m_precision);  }

    precision_t prec() const       { return m_precision; }  // for compatibility
    precision_t precision() const  { return m_precision; }

protected:

    template<typename IntType>
    static precision_t maxPrecision()    { throw std::runtime_error("DecimalPrecision::maxPrecision not implemented"); }

    template<> static precision_t maxPrecision<std::int32_t >() { return  9; }
    template<> static precision_t maxPrecision<std::uint32_t>() { return  9; }
    template<> static precision_t maxPrecision<std::int64_t >() { return 18; }
    template<> static precision_t maxPrecision<std::uint64_t>() { return 18; }


    static std::uint64_t* getPowers10Table()
    {
        static std::uint64_t _[] = 
        { 1                             //  0
        , 10                            //  1
        , 100                           //  2
        , 1000                          //  3
        , 10000                         //  4
        , 100000                        //  5
        , 1000000                       //  6
        , 10000000                      //  7
        , 100000000                     //  8
        , 1000000000                    //  9 - max for int32_t
        , 10000000000                   // 10
        , 100000000000                  // 11
        , 1000000000000                 // 12
        , 10000000000000                // 13
        , 100000000000000               // 14
        , 1000000000000000              // 15
        , 10000000000000000             // 16
        , 100000000000000000            // 17
        , 1000000000000000000           // 18 - max for int64_t
        };
    
        return &_[0];
    }


}; // class DecimalPrecision

//----------------------------------------------------------------------------



#define MARTY_DECIMAL_IMPLEMENT_RELATIONAL_OPERATORS( typeT2 )\
                                                              \
    bool operator<( const typeT2 v2 ) const                   \
    {                                                         \
        return compare(v2)<0;                                 \
    }                                                         \
                                                              \
    bool operator<=( const typeT2 v2 ) const                  \
    {                                                         \
        return compare(v2)<=0;                                \
    }                                                         \
                                                              \
    bool operator>( const typeT2 v2 ) const                   \
    {                                                         \
        return compare(v2)>0;                                 \
    }                                                         \
                                                              \
    bool operator>=( const typeT2 v2 ) const                  \
    {                                                         \
        return compare(v2)>=0;                                \
    }                                                         \
                                                              \
    bool operator==( const typeT2 v2 ) const                  \
    {                                                         \
        return compare(v2)==0;                                \
    }                                                         \
                                                              \
    bool operator!=( const typeT2 v2 ) const                  \
    {                                                         \
        return compare(v2)!=0;                                \
    }





//----------------------------------------------------------------------------
class Decimal;

class DecimalDenumerator
{

public:

    typedef DecimalPrecision::denum_t       denum_t    ;
    typedef DecimalPrecision::sdenum_t      sdenum_t   ;
    typedef DecimalPrecision::precision_t   precision_t;

    friend class Decimal;

    DecimalDenumerator( const DecimalPrecision dp )
    : m_precision( dp.prec() )
    , m_denum    ( dp.denum() )
    {}

    DecimalDenumerator( const DecimalDenumerator &dd )
    : m_precision( dd.m_precision )
    , m_denum    ( dd.m_denum     )
    {}

    DecimalDenumerator& operator=( const DecimalDenumerator &dd )
    {
        m_precision = dd.m_precision;
        m_denum     = dd.m_denum    ;
        return *this;
    }

    precision_t prec       () const { return m_precision; } // for compatibility
    precision_t precision  () const { return m_precision; }

    denum_t     denum      () const { return m_denum; }     // for compatibility
    denum_t     denumerator() const { return m_denum; }

    DecimalPrecision decimalPrecision( ) const { return DecimalPrecision(m_precision); }

    void swap( DecimalDenumerator &d2 );

    int compare( const DecimalDenumerator d2 ) const;

    MARTY_DECIMAL_IMPLEMENT_RELATIONAL_OPERATORS(DecimalDenumerator)


protected:

    void incPrec();
    void decPrec();
    denum_t precisionExpandTo( precision_t p );
    denum_t precisionShrinkTo( precision_t p );
    sdenum_t precisionFitTo( precision_t p );
    void tryIncDenum();
    void tryDecDenum();

    // default constructor disabled
    DecimalDenumerator();

    precision_t  m_precision;
    denum_t      m_denum;

}; // class DecimalDenumerator

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
void swap( DecimalDenumerator &d1, DecimalDenumerator &d2 )
{
    d1.swap( d2 );
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
class Decimal
{


public:

    //------------------------------
    typedef DecimalNumeratorRawType             num_t      ;
    typedef DecimalNumeratorUnsignedRawType     unum_t     ;
    
    typedef DecimalDenumerator::denum_t         denum_t    ;
    typedef DecimalDenumerator::sdenum_t        sdenum_t   ;
    typedef DecimalDenumerator::precision_t     precision_t;

    typedef DecimalDenumerator                  DenumeratorType;

    //------------------------------
    friend std::string decimalToString  ( Decimal d, precision_t p );
    friend Decimal     decimalFromString( const std::string &d );
    friend void swap( Decimal &d1, Decimal &d2 );

    //------------------------------
    enum class RoundingMethod
    {
        // https://en.wikipedia.org/wiki/Rounding


        // Directed rounding to an integer methods

        roundDown                                     ,
        roundFloor        = roundDown                 ,

        roundUp                                       ,
        roundCeil         = roundUp                   ,

        roundTowardsZero                              ,
        roundAwayFromInf  = roundTowardsZero          ,
        roundTrunc        = roundTowardsZero          ,

        roundTowardsInf                               ,
        roundAwayFromZero = roundTowardsInf           ,


        // Rounding to the nearest integer

        roundHalfUp                                   ,
        roundHalfTowardsPositiveInf = roundHalfUp     ,

        roundHalfDown                                 ,
        roundHalfTowardsNegativeInf = roundHalfDown   ,

        roundHalfTowardsZero                          ,
        roundHalfAwayFromInf  = roundHalfTowardsZero  ,

        roundHalfTowardsInf                           ,
        roundHalfAwayFromZero = roundHalfTowardsInf   ,
        roundMath             = roundHalfTowardsInf   ,

        roundHalfToEven                               ,
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
    static precision_t maxPrecision()    { return DecimalPrecision::maxPrecision<num_t>(); }
    precision_t precision() const        { return m_denum.precision(); }
    unum_t getPositiveNumerator() const  { return (unum_t)( (m_num<0) ? -m_num : m_num ); }

    //------------------------------
    //! Возвращает максимальный десятичный множитель (его степень), на который может быть умножено число без переполнения
    precision_t findMaxDecimalScalePower() const;

    //------------------------------
    static Decimal fromString( const std::string &s )     { return decimalFromString(s); }

    std::string   toString( precision_t p = 2 ) const     { return decimalToString( *this, p ); }
    int           toInt() const                           { return (int)(m_num/m_denum.denum()); }
    unsigned      toUnsigned() const                      { return (unsigned)(m_num/m_denum.denum()); }
    std::int64_t  toInt64() const                         { return (m_num/m_denum.denum()); }
    std::uint64_t toUnsigned64() const                    { return (std::uint64_t)(m_num/m_denum.denum()); }
    float         toFloat() const                         { return (float)((double)m_num/(double)m_denum.denum()); }
    double        toDouble() const                        { return ((double)m_num/(double)m_denum.denum()); }
    //------------------------------



    //------------------------------
    Decimal()                   : m_num(0)      , m_denum(DecimalPrecision(0))   {}
    Decimal( const Decimal &d ) : m_num(d.m_num), m_denum(d.m_denum)             {}
    Decimal( int            v, const DecimalPrecision &prec = DecimalPrecision(0) ) : m_num(v), m_denum(prec) { m_num *= m_denum.denum(); }
    Decimal( unsigned       v, const DecimalPrecision &prec = DecimalPrecision(0) ) : m_num(v), m_denum(prec) { m_num *= m_denum.denum(); }
    Decimal( std::int64_t   v, const DecimalPrecision &prec = DecimalPrecision(0) ) : m_num(v), m_denum(prec) { m_num *= m_denum.denum(); }
    Decimal( std::uint64_t  v, const DecimalPrecision &prec = DecimalPrecision(0) ) : m_num(v), m_denum(prec) { m_num *= m_denum.denum(); }
    Decimal( float          f, const DecimalPrecision &prec = DecimalPrecision(0) ) : m_num(0), m_denum(prec) { fromFloat( f, prec ); }
    Decimal( double          f, const DecimalPrecision &prec = DecimalPrecision(0) ): m_num(0), m_denum(prec) { fromFloat( f, prec ); }
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



    //----------------------------------------------------------------------------

    MARTY_DECIMAL_IMPLEMENT_RELATIONAL_OPERATORS(Decimal)

    //----------------------------------------------------------------------------



protected:


    //------------------------------
    static
    Decimal fromRawNumPrec( num_t num, precision_t prec )
    {
        Decimal res;

        res.m_num   = num;
        res.m_denum = DecimalPrecision(prec);

        return res;
    }


public:

    //------------------------------
    Decimal minimizePrecision() const;
    Decimal& minimizePrecisionInplace();
    Decimal expantPrecisionTo( precision_t p ) const;
    Decimal shrinkPrecisionTo( precision_t p ) const;
    Decimal fitPrecisionTo   ( precision_t p ) const;

    //------------------------------
    int sign() const;
    Decimal abs() const;
    Decimal mod( Decimal d2 ) const;

    //------------------------------
    Decimal getPercentOf( Decimal d ) const;
    Decimal getPermilleOf( Decimal d ) const;
    Decimal rounded( precision_t precision, RoundingMethod roundingMethod ) const;

    //------------------------------

protected:



    //------------------------------
    Decimal( num_t n, DenumeratorType denum) : m_num(n), m_denum(denum)   {}

    //------------------------------
    static
    void adjustPrecisionsToGreater( Decimal &d1, Decimal &d2 )
    {
        int cmp = d1.m_denum.compare( d2.m_denum );
        if (cmp==0) return;

        if (cmp<0)
           d1.precisionExpandTo(d2.m_denum.prec());
        else
           d2.precisionExpandTo(d1.m_denum.prec());
    }

    //------------------------------
    std::string rtrimZeros( std::string s )
    {
        if (s.empty())
            return s;

        {
            std::string::size_type commaPos = s.find( ',' );
            if (commaPos!=std::string::npos)
                s[commaPos] = '.';
        }
        
        std::string::size_type dotPos = s.find( '.' );

        if (dotPos==std::string::npos)
            return s; // do nothing

        std::string::size_type lastGoodPos = s.size()-1;

        while( lastGoodPos!=dotPos && s[lastGoodPos]=='0')
            --lastGoodPos;

        if (s[lastGoodPos]=='.')
            --lastGoodPos;

        s.erase(lastGoodPos+1);

        if (s.empty())
            s = "0";

        /*
        while( !s.empty() && s.back()=='0' ) s.erase(s.size()-1, 1);
        if ( !s.empty() && (s.back()=='.' || s.back()==',') )
            s.append(1,'0');
        */

        return s;
    }

    //------------------------------
    template<typename FloatType>
    void fromFloat( FloatType f, precision_t p )
    {
        bool precAuto = (p==0);
        *this = decimalFromString(rtrimZeros(std::to_string(f)));
        if (!precAuto)
            precisionFitTo(p);
    }

    //------------------------------
    template<typename FloatType>
    void fromFloat( FloatType f, DecimalPrecision p )
    {
        fromFloat( f, p.prec() );
    }

    //------------------------------
    int compare( num_t n ) const
    {
        if (m_num<n) return -1;
        if (m_num>n) return  1;
        return 0;
    }

    //------------------------------
    void precisionExpandTo( precision_t p )
    {
        denum_t adjust = m_denum.precisionExpandTo(p);
        m_num *= adjust;
    }

    //------------------------------
    void precisionShrinkTo( precision_t p )
    {
        denum_t adjust = m_denum.precisionShrinkTo(p);
        m_num /= adjust;
    }

    //------------------------------
    void precisionFitTo( precision_t p )
    {
        sdenum_t adjust = m_denum.precisionFitTo(p);
        if (adjust<0)
           m_num /= (denum_t)-adjust;
        else
           m_num *= (denum_t)adjust;
    }

    //------------------------------
    Decimal& minimizePrecisionImpl()
    {
        while( ((m_num%10)==0) && (m_denum.prec()>0) )
        {
            m_num /= 10;
            m_denum.decPrec();
        }

        return *this;
    }

    //------------------------------
    unum_t getLowestDecimalDigit() const
    {
        unum_t unum = getPositiveNumerator();
        return unum % 10;
    }

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
    void replaceLowestDecimalDigit( unum_t d )
    {
        // Clear place for new lowest digit
        m_num /= 10;
        m_num *= 10;

        if (d>9)
            d = 9;

        if (m_num<0)
            m_num -= (num_t)d;
        else
            m_num += (num_t)d;
    }

    Decimal makeMinimalPrecisionImplHelper( num_t newNum ) const
    {
        Decimal res;
        res.m_denum = m_denum;
        res.m_num   = newNum;
        return res;
    }

    Decimal makeMinimalPrecisionOne() const
    {
        return makeMinimalPrecisionImplHelper(1);
    }

    Decimal makeMinimalPrecisionTwo() const
    {
        return makeMinimalPrecisionImplHelper(2);
    }

    Decimal makeMinimalPrecisionFive() const
    {
        return makeMinimalPrecisionImplHelper(5);
    }


    //------------------------------

    #if 0
    enum class RoundingMethod
    {
        // https://en.wikipedia.org/wiki/Rounding


        // Directed rounding to an integer methods

        roundDown                                     ,
        roundFloor        = roundDown                 ,

        roundUp                                       ,
        roundCeil         = roundUp                   ,

        roundTowardsZero                              ,
        roundAwayFromInf  = roundTowardsZero          ,
        roundTrunc        = roundTowardsZero          ,

        roundTowardsInf                               ,
        roundAwayFromZero = roundTowardsInf           ,


        // Rounding to the nearest integer

        roundHalfUp                                   ,
        roundHalfTowardsPositiveInf = roundHalfUp     ,

        roundHalfDown                                 ,
        roundHalfTowardsNegativeInf = roundHalfDown   ,

        roundHalfTowardsZero                          ,
        roundHalfAwayFromInf  = roundHalfTowardsZero  ,

        roundHalfTowardsInf                           ,
        roundHalfAwayFromZero = roundHalfTowardsInf   ,

        roundHalfToEven                               ,
        roundBankers     = roundHalfToEven            ,
        roundBanking     = roundHalfToEven            ,

        roundHalfToOdd

    };
    #endif


    //------------------------------
    Decimal& roundingImpl( precision_t requestedPrecision, RoundingMethod roundingMethod )
    {
        // Нужно привести точность к precision+1
        // Если необходимо, то расширить точность
        // Нужно проверить, возможно ли такое расширение

        // Если невозможно расширение точности, 
        // то: а) вернуть, как есть
        //     б) кинуть исключение
        // Наверное - (а), так как если точность меньше требуемой, то и округления никакого не нужно

        // В итоге - если текущая точность меньше или равна запрошенной, то ничего делать и не надо 

        //precision_t curPrecision = m_denum.precision();
        if ( m_denum.precision() <= requestedPrecision )
            return *this;

        // if (findMaxDecimalScalePower()<1) // maxAllowedPrecisionIncrement < 1
        //     return *this;

        // Decimal makeMinimalPrecisionOne()
        // Decimal makeMinimalPrecisionFive()
        // unum_t  getLowestDecimalDigit()


        switch(roundingMethod)
        {
            case RoundingMethod::roundDown: // roundFloor
                 {
                     precisionFitTo(requestedPrecision + 1);

                     unum_t ldd = getLowestDecimalDigit();
                     precisionShrinkTo( requestedPrecision );

                     if (ldd==0)
                         break;

                     if (sign()<0)
                     {
                         *this -= makeMinimalPrecisionOne();
                     }
                 }
                 break;
        
            case RoundingMethod::roundUp: // roundCeil
                 {
                     precisionFitTo(requestedPrecision + 1);
                     unum_t ldd = getLowestDecimalDigit();
                     precisionShrinkTo( requestedPrecision );

                     if (ldd==0)
                         break;

                     if (sign()>0)
                     {
                         *this += makeMinimalPrecisionOne();
                     }
                 }
                 break;
        
            case RoundingMethod::roundTowardsZero: // roundAwayFromInf, roundTrunc
                 {
                     precisionFitTo(requestedPrecision + 1);
                     precisionShrinkTo( requestedPrecision );
                 }
                 break;
        
            case RoundingMethod::roundTowardsInf: // roundAwayFromZero
                 {
                     precisionFitTo(requestedPrecision + 1);
                     precisionShrinkTo( requestedPrecision );
                     *this += makeMinimalPrecisionOne() * sign();
                 }
                 break;
        
            case RoundingMethod::roundHalfUp: // roundHalfTowardsPositiveInf
                 {
                     precisionFitTo(requestedPrecision + 1);
                     *this += makeMinimalPrecisionFive();
                     precisionShrinkTo( requestedPrecision );
                 }
                 break;
        
            case RoundingMethod::roundHalfDown: // roundHalfTowardsNegativeInf
                 {
                     precisionFitTo(requestedPrecision + 1);
                     *this -= makeMinimalPrecisionFive();
                     precisionShrinkTo( requestedPrecision );
                 }
                 break;
        
            case RoundingMethod::roundHalfTowardsZero: // roundHalfAwayFromInf
                 {
                     precisionFitTo(requestedPrecision + 1);
                     *this -= makeMinimalPrecisionFive() * sign();
                     precisionShrinkTo( requestedPrecision );
                 }
                 break;
        
            case RoundingMethod::roundHalfTowardsInf: // roundHalfAwayFromZero, roundMath
                 {
                     precisionFitTo(requestedPrecision + 1);
                     *this += makeMinimalPrecisionFive() * sign();
                     precisionShrinkTo( requestedPrecision );
                 }
                 break;
        
            case RoundingMethod::roundHalfToEven: // roundBankers, roundBanking
                 {
                     precisionFitTo(requestedPrecision + 1);
                     precisionFitTo(requestedPrecision);

                     int     thisSign     = sign();
                     Decimal thisAbs      = abs();
                     Decimal thisAbsMod2  = abs();
                     thisAbsMod2 = thisAbsMod2.mod(makeMinimalPrecisionTwo());
                     *this = (thisAbs+thisAbsMod2) * thisSign;

                 }
                 break;
        
            case RoundingMethod::roundHalfToOdd:
                 {
                     throw std::runtime_error("RoundingMethod::roundHalfToOdd not implemented");
                 }
                 break;
        
        
        };



        // precisionExpandTo( requestedPrecision + 1 ); // На всякий случай расширяем точность
        // precisionShrinkTo( requestedPrecision + 1 ); // Обрезаем до требуемоей + 1 (если изначально была больше)



        return *this;
    }

    //----------------------------------------------------------------------------





    //----------------------------------------------------------------------------
    // Class members goes here

    num_t               m_num;
    DenumeratorType     m_denum;


}; // class Decimal

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
#include "marty_decimal_impl.h"
//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
#define MARTY_DECIMAL_IMPLEMENT_FRIEND_OVERLOADS_FOR_INTEGRAL_TYPE( integralType )                                          \
                                                                                                                            \
    inline bool operator< ( integralType i, const Decimal &d ) { return Decimal(i) <  d; }                                  \
    inline bool operator<=( integralType i, const Decimal &d ) { return Decimal(i) <= d; }                                  \
    inline bool operator> ( integralType i, const Decimal &d ) { return Decimal(i) >  d; }                                  \
    inline bool operator>=( integralType i, const Decimal &d ) { return Decimal(i) >= d; }                                  \
    inline bool operator==( integralType i, const Decimal &d ) { return Decimal(i) == d; }                                  \
    inline bool operator!=( integralType i, const Decimal &d ) { return Decimal(i) != d; }                                  \
                                                                                                                            \
    inline Decimal operator + ( integralType i, const Decimal &d ) { return Decimal(i).operator+ ( d ); }                   \
    inline Decimal operator - ( integralType i, const Decimal &d ) { return Decimal(i).operator- ( d ); }                   \
    inline Decimal operator * ( integralType i, const Decimal &d ) { return Decimal(i).operator* ( d ); }                   \
    inline Decimal operator / ( integralType i, const Decimal &d ) { return Decimal(i).operator/ ( d ); }                   \
    inline Decimal operator % ( integralType i, const Decimal &d ) { return Decimal(i).operator% ( d ); }

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
MARTY_DECIMAL_IMPLEMENT_FRIEND_OVERLOADS_FOR_INTEGRAL_TYPE( int           )
MARTY_DECIMAL_IMPLEMENT_FRIEND_OVERLOADS_FOR_INTEGRAL_TYPE( unsigned      )
MARTY_DECIMAL_IMPLEMENT_FRIEND_OVERLOADS_FOR_INTEGRAL_TYPE( std::int64_t  )
MARTY_DECIMAL_IMPLEMENT_FRIEND_OVERLOADS_FOR_INTEGRAL_TYPE( std::uint64_t )
MARTY_DECIMAL_IMPLEMENT_FRIEND_OVERLOADS_FOR_INTEGRAL_TYPE( float         )
MARTY_DECIMAL_IMPLEMENT_FRIEND_OVERLOADS_FOR_INTEGRAL_TYPE( double        )

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
void swap( Decimal &d1, Decimal &d2 )
{
    d1.swap(d2);
}

//----------------------------------------------------------------------------
inline
std::string decimalToString  ( Decimal d, Decimal::precision_t p = 2 )
{
    d = d.expantPrecisionTo( p );

    Decimal::num_t p1 = d.m_num / d.m_denum.denum();
    Decimal::num_t p2 = d.m_num % d.m_denum.denum();

    std::string res = std::to_string(p1);

    std::size_t prec = d.m_denum.prec();

    if (!prec)
        return res; // No decimal digits after dot

    if (p2<0)
        p2 = -p2;

    std::string strP2 = std::to_string(p2);

    std::size_t leadingZerosNum = 0;
    if (prec>strP2.size())
        leadingZerosNum = prec - strP2.size();

    return res + std::string(1, '.') + std::string(leadingZerosNum, '0') + strP2;
}

//----------------------------------------------------------------------------
// For compatibility with old code
inline
std::string toString  ( const Decimal     &d, Decimal::precision_t p = 2 )
{
    return decimalToString(d,p);
}

//----------------------------------------------------------------------------
template<typename CharType>
inline
int charToDecimalDigit( CharType ch )
{
    if (ch<(CharType)'0') return -1;
    if (ch>(CharType)'9') return -1;
    return (int)(ch - (CharType)'0');
}

//----------------------------------------------------------------------------
template<typename CharType>
inline
bool charIsDecimalSeparator( CharType ch )
{
    if (ch==(CharType)'.') return true;
    if (ch==(CharType)',') return true;
    return false;
}

//----------------------------------------------------------------------------
template<typename CharType>
inline
int charIsSpaceOrTab( CharType ch )
{
    switch((char)ch)
    {
        case ' ' : return true;
        case '\t': return true;
        default  : return false;
    };
}

//----------------------------------------------------------------------------
inline
bool isDecimalStringCandidateExactPlainDecimalValueChar( char ch )
{
    switch(ch)
    {
        case '0': case '1': case '2': case '3': case '4':            
        case '5': case '6': case '7': case '8': case '9':
            return true; // good char - this is a digit

        case ',': case '.':
            return true; // good char - this is a decimal separator

        case ' ': case '\'': case '`':
            return true; // good char - this is a group separator

        case '+': case '-':
            return true; // nice signum sign

        default: return false;
    }

    return true;
}

//----------------------------------------------------------------------------
inline
bool isDecimalStringCandidateExactPlainDecimalValueString( const std::string &numberStr )
{
    typedef std::string::size_type sz_t;
    sz_t pos = 0, sz = numberStr.size();

    for(; pos!=sz; ++pos)
    {
        if (!isDecimalStringCandidateExactPlainDecimalValueChar(numberStr[pos]))
            return false;
    }

    return true;
}

//----------------------------------------------------------------------------
inline
char decimalStringPrepareConvertChar( char ch )
{
    if (ch==' ' || ch=='\'' || ch=='`') return 0;
    if (ch==',' ) return '.';
    return ch;
}

//----------------------------------------------------------------------------
inline
std::string decimalStringPrepareForConvert( const std::string &numberStr )
{
    typedef std::string::size_type sz_t;
    sz_t pos = 0, sz = numberStr.size();
    std::string res;

    for(; pos!=sz; ++pos)
    {
        char ch = decimalStringPrepareConvertChar(numberStr[pos]);
        if (ch)
           res.append(1,ch);
    }

    return res;
}

//----------------------------------------------------------------------------
inline
Decimal     decimalFromString( const std::string &numberStr_ )
{
    std::string numberStr = decimalStringPrepareForConvert(numberStr_); // change ',' to '.' and skip thousands separator

    typedef std::string::size_type sz_t;
    sz_t pos = 0, sz = numberStr.size();

    if (!isDecimalStringCandidateExactPlainDecimalValueString(numberStr))
    {
        // may be double in exponential format
        // Лень самому парсить
        sz_t cnt = 0;
        double dbl = std::stod( numberStr, &cnt );

        if (cnt!=sz)
            throw std::runtime_error("Decimal fromString - possible exponential form parsing failed");

        return Decimal(dbl);
    }


    bool neg = false;

    while( charIsSpaceOrTab(numberStr[pos]) && pos!=sz ) pos++;
    if (pos==sz) throw std::runtime_error("Decimal fromString - empty string taken as number string");

    if (numberStr[pos]=='+')                        { pos++; }
    else if (numberStr[pos]=='-')                   { pos++; neg = true; }
    else if (charToDecimalDigit(numberStr[pos])>=0) {}
    else throw std::runtime_error("Decimal fromString - invalid character found");

    while( charIsSpaceOrTab(numberStr[pos]) ) pos++;

    std::int64_t  num = 0;
    std::uint32_t precision = 0;

    int dig = charToDecimalDigit(numberStr[pos]);
    while(dig>=0)
    {
        num *= 10;
        num += (std::int64_t)dig;
        pos++;
        dig = charToDecimalDigit(numberStr[pos]);
    }

    if (!charIsDecimalSeparator(numberStr[pos]))
    {
        if (neg)
            num = -num;

        return Decimal( num, DecimalPrecision(0) );
    }

    pos++;

    unsigned prec = 0;

    dig = charToDecimalDigit(numberStr[pos]);
    while( sz != pos && (dig>=0  /* && (d[pos]!=' ' && d[pos]!='\'') */ ) )
    {
        num *= 10;
        num += (std::int64_t)dig;
        pos++;
        dig = charToDecimalDigit(numberStr[pos]);
        prec++;
    }

    if (neg)
        num = -num;

    return Decimal::fromRawNumPrec( num, prec );

}

//----------------------------------------------------------------------------
// For compatibility with old code
inline
Decimal     fromString( const std::string &numberStr_ )
{
    return decimalFromString(numberStr_);
}


//----------------------------------------------------------------------------
inline
std::ostream& operator<<( std::ostream& os, const Decimal &v )
{
    auto minPrecision = (Decimal::precision_t)os.precision();
    if (minPrecision<1)
        minPrecision = 1;
    os << v.toString(minPrecision);
    return os;
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------

} // namespace marty


