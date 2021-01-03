#pragma once


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



//----------------------------------------------------------------------------
// 4 294 967 295 uint32_t max
// 1 000 000 000 - must be enough for money or use more wide type in typedef below

typedef std::uint32_t  DecimalDenumeratorRawType;
typedef std::int32_t   DecimalDenumeratorSignedRawType;

//----------------------------------------------------------------------------
//  9 223 372 036 854 775 808
//  1 000 000 000.000 000  000 - 10**9 -  миллиард - бюджет страны верстать не выйдет с точностью 9 нулей после точки
//  1 000 000 000 000 00.0 000 - 10**15 - 100 трлн с точностью до сотых копейки - для домашней бухгалтерии норм

typedef std::int64_t   DecimalNumeratorRawType;



//----------------------------------------------------------------------------
class DecimalDenumerator;

class DecimalPrecision
{
    friend class DecimalDenumerator;

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

    denum_t denum( ) const
    {
        return denum(m_precision);
    }

    precision_t prec() const 
    {
        return m_precision;
    }

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

    precision_t prec () const { return m_precision; }
    denum_t     denum() const { return m_denum; }

    DecimalPrecision decimalPrecision( ) const { return DecimalPrecision(m_precision); }

    void swap( DecimalDenumerator &d2 )
    {
        //using namespace std;
        std::swap( m_precision, d2.m_precision );
        std::swap( m_denum    , d2.m_denum     );
    }

    int compare( const DecimalDenumerator d2 ) const
    {
        if (m_precision<d2.m_precision) return -1;
        if (m_precision>d2.m_precision) return  1;
        return 0;
    }

    MARTY_DECIMAL_IMPLEMENT_RELATIONAL_OPERATORS(DecimalDenumerator)


protected:

    void incPrec()
    {
        tryIncDenum();
        m_precision += 1;
    }

    void decPrec()
    {
        tryDecDenum();
        m_precision -= 1;
    }

    denum_t expandTo( precision_t p )
    {
        denum_t deltaDenum = 1;
        while(m_precision < p)
        {
            incPrec();
            deltaDenum *= 10;
        }

        return deltaDenum;
    }

    denum_t shrinkTo( precision_t p )
    {
        denum_t deltaDenum = 1;
        while(m_precision > p)
        {
            decPrec();
            deltaDenum *= 10;
        }

        return deltaDenum;
    }

    sdenum_t fitTo( precision_t p )
    {
        if (m_precision<p)
            return   (sdenum_t)expandTo(p);
        else
            return - (sdenum_t)shrinkTo(p);
    }


    void tryIncDenum()
    {
        denum_t newDenum = m_denum*10u;
        if (newDenum<m_denum)
            throw std::runtime_error("DecimalDenumerator precision to big to increment");
        m_denum = newDenum;
    }

    void tryDecDenum()
    {
        if (!m_denum)
            throw std::runtime_error("DecimalDenumerator precision to small to decrement");
        m_denum = m_denum / 10u;
    }


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
std::string toString  ( const Decimal     &d );
Decimal     fromString( const std::string &d );
void swap( Decimal &d1, Decimal &d2 );

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
class Decimal
{

    friend std::string toString  ( const Decimal     &d );
    friend Decimal     fromString( const std::string &d );
    friend void swap( Decimal &d1, Decimal &d2 );


public:

    typedef DecimalNumeratorRawType           num_t      ;
    typedef DecimalDenumerator::denum_t       denum_t    ;
    typedef DecimalDenumerator::sdenum_t      sdenum_t   ;
    typedef DecimalDenumerator::precision_t   precision_t;

    typedef DecimalDenumerator                DenumeratorType;

    Decimal()                   : m_num(0)      , m_denum(DecimalPrecision(0))   {}
    Decimal( const Decimal &d ) : m_num(d.m_num), m_denum(d.m_denum)             {}
    Decimal( int            v, const DecimalPrecision &prec = DecimalPrecision(0) ) : m_num(v), m_denum(prec) { m_num *= m_denum.denum(); }
    Decimal( unsigned       v, const DecimalPrecision &prec = DecimalPrecision(0) ) : m_num(v), m_denum(prec) { m_num *= m_denum.denum(); }
    Decimal( std::int64_t   v, const DecimalPrecision &prec = DecimalPrecision(0) ) : m_num(v), m_denum(prec) { m_num *= m_denum.denum(); }
    Decimal( std::uint64_t  v, const DecimalPrecision &prec = DecimalPrecision(0) ) : m_num(v), m_denum(prec) { m_num *= m_denum.denum(); }

    Decimal( float          f, const DecimalPrecision &prec = DecimalPrecision(0) )
    : m_num(0), m_denum(prec)
    {
        fromFloat( f, prec );
    }

    Decimal( double          f, const DecimalPrecision &prec = DecimalPrecision(0) )
    : m_num(0), m_denum(prec)
    {
        fromFloat( f, prec );
    }

    void swap( Decimal &d2 )
    {
        //using namespace std;
        std::swap( m_num   , d2.m_num   );
        m_denum.swap( d2.m_denum );
    }


    Decimal& operator=( Decimal d2 )
    {
        swap(d2);
        return *this;
    }

    // операторы преобразования типа 

    explicit operator int() const
    {
        return (int)(m_num/m_denum.denum());
    }

    explicit operator unsigned() const
    {
        return (unsigned)(m_num/m_denum.denum());
    }

    explicit operator std::int64_t() const
    {
        return (m_num/m_denum.denum());
    }

    explicit operator std::uint64_t() const
    {
        return (std::uint64_t)(m_num/m_denum.denum());
    }

    explicit operator float() const
    {
        return (float)((double)m_num/(double)m_denum.denum());
    }

    explicit operator double() const
    {
        return ((double)m_num/(double)m_denum.denum());
    }


    Decimal operator + ( Decimal d2 ) const
    {
        Decimal d1 = *this;
        adjustPrecisions( d1, d2 );
        d1.m_num += d2.m_num;
        return d1;
    }

    Decimal operator - ( Decimal d2 ) const
    {
        Decimal d1 = *this;
        adjustPrecisions( d1, d2 );
        d1.m_num -= d2.m_num;
        return d1;
    }

    Decimal operator * ( Decimal d2 ) const
    {
        //minimizePrecision();
        d2.minimizePrecision();

        num_t num = m_num * d2.m_num;

        if (m_denum < d2.m_denum)
        {
            num /= m_denum.denum();
            return Decimal( num, d2.m_denum ).minimizePrecision();
        }
        else
        {
            num /= d2.m_denum.denum();
            return Decimal( num, m_denum ).minimizePrecision();
        }
    }

    // 1200.00000 / 22.000 = 54.5454545454545
    // 1200 00000 / 22 000 = 5454.54545454
    Decimal operator / ( Decimal d2 ) const
    {
        d2.minimizePrecision();

        Decimal d1 = *this;
        d1.expandTo( m_denum.prec() + d2.m_denum.prec() );
        return Decimal( d1.m_num / d2.m_num, m_denum);
    }

    // 1200.00000   % 22.000    = 12.000
    // 1200         % 22        = 12
    // 1200 00000   % 22 000    = 12000
    // 1200 000000  % 22 000    = 10000
    // 1200.00000   % 22.00000  = 12.00000
    // 120000000    % 2200000   = 12 00000

    Decimal operator % ( Decimal d2 ) const
    {
        Decimal d1 = *this;
        adjustPrecisions( d1, d2 );
        d1.m_num %= d2.m_num;
        return d1;
    }


    Decimal& operator += ( Decimal d2 )
    {
        *this = operator+(d2);
        return *this;
    }

    Decimal& operator -= ( Decimal d2 )
    {
        *this = operator-(d2);
        return *this;
    }

    Decimal& operator *= ( Decimal d2 )
    {
        *this = operator*(d2);
        return *this;
    }

    Decimal& operator /= ( Decimal d2 )
    {
        *this = operator/(d2);
        return *this;
    }

    Decimal& operator %= ( Decimal d2 )
    {
        *this = operator%(d2);
        return *this;
    }

    // Decimal operator +   ( int i ) const { return operator+ ( Decimal((int)i, DecimalPrecision(0)) ); }
    // Decimal& operator += ( int i )       { return operator+=( Decimal((int)i, DecimalPrecision(0)) ); }


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



    int compare( Decimal d2 ) const
    {
        if (m_denum.prec()==d2.m_denum.prec())
        {
            return compare(d2.m_num);
        }

        Decimal d1 = *this;
        adjustPrecisions( d1, d2 );
        return d1.compare(d2);
    }

    MARTY_DECIMAL_IMPLEMENT_RELATIONAL_OPERATORS(Decimal)


    static
    Decimal fromRawNumPrec( num_t num, precision_t prec )
    {
        Decimal res;

        res.m_num   = num;
        res.m_denum = DecimalPrecision(prec);

        return res;
    }


protected:


    Decimal( num_t n, DenumeratorType denum) : m_num(n), m_denum(denum)   {}

    static
    void adjustPrecisions( Decimal &d1, Decimal &d2 )
    {
        int cmp = d1.m_denum.compare( d2.m_denum );
        if (cmp==0) return;

        if (cmp<0)
           d1.expandTo(d2.m_denum.prec());
        else
           d2.expandTo(d1.m_denum.prec());
    }

    std::string rtrimZeros( std::string s )
    {
        while( !s.empty() && s.back()=='0' ) s.erase(s.size()-1, 1);
        if ( !s.empty() && (s.back()=='.' || s.back()==',') )
            s.append(1,'0');
        return s;
    }

    template<typename FloatType>
    void fromFloat( FloatType f, precision_t p )
    {
        bool precAuto = (p==0);
        *this = fromString(rtrimZeros(std::to_string(f)));
        if (!precAuto)
            fitTo(p);
    }

    template<typename FloatType>
    void fromFloat( FloatType f, DecimalPrecision p )
    {
        fromFloat( f, p.prec() );
    }

    int compare( num_t n ) const
    {
        if (m_num<n) return -1;
        if (m_num>n) return  1;
        return 0;
    }

    void expandTo( precision_t p )
    {
        denum_t adjust = m_denum.expandTo(p);
        m_num *= adjust;
    }

    void shrinkTo( precision_t p )
    {
        denum_t adjust = m_denum.shrinkTo(p);
        m_num /= adjust;
    }

    void fitTo( precision_t p )
    {
        sdenum_t adjust = m_denum.fitTo(p);
        if (adjust<0)
           m_num /= (denum_t)-adjust;
        else
           m_num *= (denum_t)adjust;
    }

    Decimal& minimizePrecision()
    {
        while( ((m_num%10)==0) && (m_denum.prec()>0) )
        {
            m_num /= 10;
            m_denum.decPrec();
        }

        return *this;
    }


    
    //precision_t

    //DecimalPrecision decimalPrecision( )

    num_t               m_num;
    DenumeratorType     m_denum;

}; // class Decimal

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
std::string toString  ( const Decimal     &d )
{
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
Decimal     fromString( const std::string &numberStr_ )
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

    if (neg)
        num = -num;

    if (!charIsDecimalSeparator(numberStr[pos]))
    {
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

    return Decimal::fromRawNumPrec( num, prec );

}

//----------------------------------------------------------------------------
inline
std::ostream& operator<<( std::ostream &s, const Decimal &d )
{
    s<<toString(d);
    return s;
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------

} // namespace marty


