#pragma once


#if !defined(MARTY_DECIMAL_H__079F0131_B01B_44ED_BF0F_8DFECAB67FD7__)
    #error "This header file must be used from marty_decimal.h only"
#endif


//----------------------------------------------------------------------------
inline
void Decimal::assignFromString( const char        *pStr )
{
    const char *pStrOrg = pStr;

    // Skip ws
    while(*pStr==' ' || *pStr=='\t') ++pStr;

    if (*pStr=='+' || *pStr=='-')
    {
        if (*pStr=='-')
            m_sign = -1;
        else
            m_sign =  1;

        // Skip ws
        while(*pStr==' ' || *pStr=='\t') ++pStr;
    }

    const char *pStrEnd = 0;

    m_precision = bcd::makeRawBcdNumber( m_number
                                       , pStr
                                       , (std::size_t)-1
                                       , &pStrEnd
                                       );

    if (*pStrEnd!=0)
    {
        throw std::runtime_error( std::string("Decimal::fromString: invalid number string: ") + pStrOrg );
    }

    if (bcd::checkForZero( m_number ))
    {
        bcd::clearShrink(m_number);
        m_sign = 0;
    }

}

//----------------------------------------------------------------------------
inline
void Decimal::assignFromString( const std::string &str  )
{
    assignFromString( str.c_str() );
}

//----------------------------------------------------------------------------
inline
void Decimal::assignFromIntImpl( std::int64_t  i, int precision )
{
    m_precision = 0;
    bcd::clearShrink(m_number);

    if (i==0)
    {
        m_sign = 0;
    }
    else if (i>0)
    {
        m_sign = 1;
        bcd::makeRawBcdNumberFromUnsigned( m_number, (std::uint64_t)i );
        m_precision = precision;
    }
    else // i<0
    {
        m_sign = -1;
        //NOTE: !!! Вообще не паримся по поводу несимметричности множества целых относительно нуля
        bcd::makeRawBcdNumberFromUnsigned( m_number, (std::uint64_t)-i ); 
        m_precision = precision;
    }

}

//----------------------------------------------------------------------------
inline
void Decimal::assignFromIntImpl( std::uint64_t u, int precision )
{
    m_precision = 0;
    bcd::clearShrink(m_number);

    if (u==0)
    {
        m_sign = 0;
    }
    else // u>0 - unsigned же
    {
        m_sign = 1;
        bcd::makeRawBcdNumberFromUnsigned( m_number, u );
        m_precision = precision;
    }

}

//----------------------------------------------------------------------------
inline
void Decimal::assignFromDoubleImpl( double d, int precision )
{
    if (precision<0)
        precision = 0;

    if (precision>12)
        precision = 12;

    // https://docs.microsoft.com/en-us/cpp/c-runtime-library/format-specification-syntax-printf-and-wprintf-functions?view=msvc-160
    // printf( "%.*f", 3, 3.14159265 ); /* 3.142 output */

    char buf[32]; // forever enough for all doubles

    sprintf( &buf[0], "%.*f", precision, d ); 

    assignFromString( &buf[0] );

}

//----------------------------------------------------------------------------
inline
const char* Decimal::toString( char *pBuf, std::size_t bufSize, int precision ) const
{
    if (bufSize < 5 )
        throw std::runtime_error("marty::Decimal::toString: bufSize is not enough");

    if (m_sign==0)
    {
        pBuf[0] = '0'; pBuf[1] = 0;
        return pBuf;
    }

    unsigned idx = 0;

    if (m_sign<0)
    {
        pBuf[idx++] = '-';
    }

    return bcd::formatRawBcdNumber( m_number, m_precision, &pBuf[idx], bufSize-1 );

}

//----------------------------------------------------------------------------
inline
std::string Decimal::toString( int precision ) const
{
    char buf[256]; // 256 знаком в числе хватит для всех :)
    return toString( &buf[0], sizeof(buf), precision );
}

//----------------------------------------------------------------------------
inline
int Decimal::compare( const Decimal &r ) const
{

    if (m_sign < r.m_sign )
        return -1;

    if (m_sign > r.m_sign )
        return  1;

    // m_sign equals

    if (m_sign==0)
        return 0; // Нули - равны

    if (m_sign>0) // Больше нуля - сравнение BCD прямое
    {
        return bcd::compareRaws( m_number, m_precision, r.m_number, r.m_precision );
    }

    // Меньше нуля - сравнение BCD инвертируется - в минусах всё инверсное по знаку
    return - bcd::compareRaws( m_number, m_precision, r.m_number, r.m_precision );

}













