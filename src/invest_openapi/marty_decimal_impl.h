#pragma once


//----------------------------------------------------------------------------
inline
void DecimalDenumerator::swap( DecimalDenumerator &d2 )
{
    std::swap( m_precision, d2.m_precision );
    std::swap( m_denum    , d2.m_denum     );
}

//----------------------------------------------------------------------------
inline
int DecimalDenumerator::compare( const DecimalDenumerator d2 ) const
{
    if (m_precision<d2.m_precision) return -1;
    if (m_precision>d2.m_precision) return  1;
    return 0;
}

//----------------------------------------------------------------------------
inline
void DecimalDenumerator::incPrec()
{
    tryIncDenum();
    m_precision += 1;
}

//----------------------------------------------------------------------------
inline
void DecimalDenumerator::decPrec()
{
    tryDecDenum();
    m_precision -= 1;
}

//----------------------------------------------------------------------------
inline
DecimalDenumerator::denum_t
DecimalDenumerator::precisionExpandTo( precision_t p )
{
    denum_t deltaDenum = 1;
    while(m_precision < p)
    {
        incPrec();
        deltaDenum *= 10;
    }

    return deltaDenum;
}

//----------------------------------------------------------------------------
inline
DecimalDenumerator::denum_t
DecimalDenumerator::precisionShrinkTo( precision_t p )
{
    denum_t deltaDenum = 1;
    while(m_precision > p)
    {
        decPrec();
        deltaDenum *= 10;
    }

    return deltaDenum;
}

//----------------------------------------------------------------------------
inline
DecimalDenumerator::sdenum_t
DecimalDenumerator::precisionFitTo( precision_t p )
{
    if (m_precision<p)
        return   (sdenum_t)precisionExpandTo(p);
    else
        return - (sdenum_t)precisionShrinkTo(p);
}

//----------------------------------------------------------------------------
inline
void DecimalDenumerator::tryIncDenum()
{
    denum_t newDenum = m_denum*10u;
    if (newDenum<m_denum)
        throw std::runtime_error("DecimalDenumerator precision to big to increment");
    m_denum = newDenum;
}

//----------------------------------------------------------------------------
inline
void DecimalDenumerator::tryDecDenum()
{
    if (!m_denum)
        throw std::runtime_error("DecimalDenumerator precision to small to decrement");
    m_denum = m_denum / 10u;
}

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
inline
Decimal Decimal::minimizePrecision() const
{
    Decimal res = *this;
    return res.minimizePrecisionImpl();
}

//----------------------------------------------------------------------------
inline
Decimal& Decimal::minimizePrecisionInplace()
{
    return minimizePrecisionImpl();
}

//----------------------------------------------------------------------------
inline
Decimal Decimal::expantPrecisionTo( precision_t p ) const
{
    Decimal res = *this;
    res.precisionExpandTo(p);
    return res;
}

//----------------------------------------------------------------------------
inline
Decimal Decimal::shrinkPrecisionTo( precision_t p ) const
{
    Decimal res = *this;
    res.precisionShrinkTo(p);
    return res;
}

//----------------------------------------------------------------------------
inline
Decimal Decimal::fitPrecisionTo( precision_t p ) const
{
    Decimal res = *this;
    res.precisionFitTo(p);
    return res;
}

//----------------------------------------------------------------------------
inline
Decimal::precision_t Decimal::findMaxDecimalScalePower() const
{
    unum_t unum = getPositiveNumerator();

    precision_t maxScalePower = maxPrecision();

    precision_t curScalePower = 0;

    for(; unum!=0; unum /= 10, ++curScalePower ) {}

    return maxScalePower - curScalePower;

}

//----------------------------------------------------------------------------

    
    
//----------------------------------------------------------------------------
inline
void Decimal::swap( Decimal &d2 )
{
    //using namespace std;
    std::swap( m_num   , d2.m_num   );
    m_denum.swap( d2.m_denum );
}

//------------------------------
inline
Decimal& Decimal::operator=( Decimal d2 )
{
    swap(d2);
    return *this;
}

//------------------------------
inline
int Decimal::compare( Decimal d2 ) const
{
    if (m_denum.prec()==d2.m_denum.prec())
    {
        return compare(d2.m_num);
    }

    Decimal d1 = *this;
    adjustPrecisionsToGreater( d1, d2 );
    return d1.compare(d2);
}
//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
inline
Decimal Decimal::operator + ( Decimal d2 ) const
{
    Decimal d1 = *this;
    adjustPrecisionsToGreater( d1, d2 );
    d1.m_num += d2.m_num;
    return d1;
}

//------------------------------
inline
Decimal Decimal::operator - ( Decimal d2 ) const
{
    Decimal d1 = *this;
    adjustPrecisionsToGreater( d1, d2 );
    d1.m_num -= d2.m_num;
    return d1;
}

//------------------------------
inline
Decimal Decimal::operator - ( ) const
{
    Decimal res = *this;
    res.m_num = -res.m_num;
    return res;
}

//------------------------------
inline
Decimal Decimal::operator * ( Decimal d2 ) const
{
    //minimizePrecisionImpl();
    d2.minimizePrecisionImpl();

    num_t num = m_num * d2.m_num;

    if (m_denum < d2.m_denum)
    {
        num /= m_denum.denum();
        return Decimal( num, d2.m_denum ).minimizePrecisionImpl();
    }
    else
    {
        num /= d2.m_denum.denum();
        return Decimal( num, m_denum ).minimizePrecisionImpl();
    }
}

//------------------------------
// 1200.00000 / 22.000 = 54.5454545454545
// 1200 00000 / 22 000 = 5454.54545454
// 1200.00000 / 25.000 = 48

inline
Decimal Decimal::divide( Decimal devider, precision_t resultPrecision ) const
{
    devider.minimizePrecisionInplace();

    precision_t maxAllowedPrecisionIncrement  = findMaxDecimalScalePower();
    precision_t maxAllowedPrecision           = this->m_denum.precision() + maxAllowedPrecisionIncrement;

    precision_t requiredSrcPrecision = resultPrecision + devider.m_denum.precision();

    // Точность результата = точность делимого - точность делителя
    // Для этого точность делимого нужно расширить до resultPrecision + точность делителя
    // Но если расширение точности невозможно, то уменьшаем точность делителя
    // Можно без цикла, но лень думать

    while( (requiredSrcPrecision>maxAllowedPrecision) && (devider.m_denum.precision()!=0) )
    {
        devider = devider.shrinkPrecisionTo( devider.m_denum.precision()-1 );
        requiredSrcPrecision = resultPrecision + devider.m_denum.precision();
    }

    if (requiredSrcPrecision>maxAllowedPrecision)
        requiredSrcPrecision = maxAllowedPrecision;

    Decimal d1 = expantPrecisionTo( requiredSrcPrecision );

    return Decimal( d1.m_num / devider.m_num, DenumeratorType(d1.m_denum.precision()-devider.m_denum.precision()) );
    //DecimalPrecision
}

//------------------------------
inline
Decimal Decimal::operator / ( Decimal d2 ) const
{
    d2.minimizePrecisionImpl();

    Decimal d1 = *this;
    d1.precisionExpandTo( m_denum.prec() + d2.m_denum.prec() );
    return Decimal( d1.m_num / d2.m_num, m_denum);
}

//------------------------------
// 1200.00000   % 22.000    = 12.000
// 1200         % 22        = 12
// 1200 00000   % 22 000    = 12000
// 1200 000000  % 22 000    = 10000
// 1200.00000   % 22.00000  = 12.00000
// 120000000    % 2200000   = 12 00000

inline
Decimal Decimal::operator % ( Decimal d2 ) const
{
    return mod(d2);
    /*
    Decimal d1 = *this;
    adjustPrecisionsToGreater( d1, d2 );
    d1.m_num %= d2.m_num;
    return d1;
    */
}

//------------------------------
inline
Decimal& Decimal::operator += ( Decimal d2 )
{
    *this = operator+(d2);
    return *this;
}

//------------------------------
inline
Decimal& Decimal::operator -= ( Decimal d2 )
{
    *this = operator-(d2);
    return *this;
}

//------------------------------
inline
Decimal& Decimal::operator *= ( Decimal d2 )
{
    *this = operator*(d2);
    return *this;
}

//------------------------------
inline
Decimal& Decimal::operator /= ( Decimal d2 )
{
    *this = operator/(d2);
    return *this;
}

//------------------------------
inline
Decimal& Decimal::operator %= ( Decimal d2 )
{
    *this = operator%(d2);
    return *this;
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
Decimal Decimal::getPercentOf( Decimal d ) const
{
    //UNDONE: !!! Need to make correct rounding
    Decimal tmp = Decimal(100) * *this;
    tmp.precisionExpandTo(2u); // точнось - сотые доли процента
    return tmp / d;
}

//------------------------------
inline
Decimal Decimal::getPermilleOf( Decimal d ) const
{
    //UNDONE: !!! Need to make correct rounding
    Decimal tmp = Decimal(1000) * *this;
    tmp.precisionExpandTo(2u); // точнось - сотые доли промилле
    return tmp / d;
}

//------------------------------
inline
Decimal Decimal::rounded( precision_t precision, RoundingMethod roundingMethod ) const
{
    Decimal res = *this;
    res.roundingImpl( precision, roundingMethod );
    return res;
}
//------------------------------
inline
int Decimal::sign() const
{
    if (m_num<0)
        return -1;

    if (m_num>0)
        return 1;

    return 0;
}

//------------------------------
inline
Decimal Decimal::abs() const
{
    Decimal res = *this;

    if (res.m_num<0)
       res.m_num = -res.m_num;

    return res;
}

//------------------------------
inline
Decimal Decimal::mod( Decimal d2 ) const
{
    d2.minimizePrecisionInplace();

    Decimal d1 = *this;
    d1.minimizePrecisionInplace();

    adjustPrecisionsToGreater( d1, d2 );

    d1.m_num %= d2.m_num;
    return d1;
}

