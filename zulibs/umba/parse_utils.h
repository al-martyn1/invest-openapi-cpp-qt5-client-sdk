#pragma once

//----------------------------------------------------------------------------

/*! \file
    \brief Утилиты для парсинга
*/

//----------------------------------------------------------------------------

#include <stdint.h>

// umba::parse_utils::

namespace umba
{



//! Утилиты для парсинга
namespace parse_utils
{

//! Проверка на нижнерегистровую букву
inline
bool isAlphaLower( char ch )
{
    if (ch>='a' && ch<='z')
        return true;
    return false;
}

//! Проверка на нижнерегистровую букву
inline
bool isAlphaLower( uint8_t ch )
{
    return isAlphaLower( (char)ch );
}

//! Проверка на верхнерегистровую букву
inline
bool isAlphaUpper( char ch )
{
    if (ch>='A' && ch<='Z')
        return true;
    return false;
}

//! Проверка на верхнерегистровую букву
inline
bool isAlphaUpper( uint8_t ch )
{
    return isAlphaUpper( (char)ch );
}

//! Проверка на букву
inline
bool isAlpha( char ch )
{
    return isAlphaLower(ch) || isAlphaUpper(ch);
}

//! Проверка на букву
inline
bool isAlpha( uint8_t ch )
{
    return isAlphaLower(ch) || isAlphaUpper(ch);
}

//! Пробел?
inline
bool isSpace( char ch )
{
    return ch==' ';
}

//! Пробельный символ?
inline
bool isWhitespace( char ch )
{
    return ch==' ' || ch=='\r' || ch=='\n' || ch=='\t';
}

//! Пробел?
inline
bool isSpace( uint8_t ch )
{
    return isSpace((char)ch);
}

//! Пробельный символ?
inline
bool isWhitespace( uint8_t ch )
{
    return isWhitespace((char)ch);
}


//! Преобразование из цифрового сивола
inline
int toDigit( char ch )
{
    if (ch>='0' && ch<='9')
        return (int)(ch-'0');
    if (ch>='a' && ch<='z')
        return (int)(ch-'a'+10);
    if (ch>='A' && ch<='Z')
        return (int)(ch-'A'+10);
    return -1;
}

//! Преобразование из цифрового сивола
inline
int toDigit( uint8_t ch )
{
    return toDigit( (char)ch );
}

//! Цифра?
inline
bool isDigit( char ch, int ss = 10)
{
    int d = toDigit( ch );
    if (d<0 || d>=ss)
        return false;
    return true;
}

//! Цифра?
inline
bool isDigit( uint8_t ch, int ss = 10)
{
    return isDigit( (char)ch, ss );
}

//! Пропустить пробелы
inline
size_t skipSpaces( const char* pBuf )
{
    for( size_t i=0; ; ++i )
    {
        if ( isSpace(pBuf[i]) ) // isSpace detects zero by itself
            continue;
        return i;
    }

    //return (size_t)-1;
}

//! Пропустить пробелы
inline
size_t skipSpaces( const uint8_t* pBuf )
{
    return skipSpaces( (const char*)pBuf );
}

//! Пропустить пробелы
inline
size_t skipSpaces( const char* pBuf, size_t bufSize )
{
    for( size_t i=0; i!=bufSize; ++i )
    {
        if ( isSpace(pBuf[i]) ) // isSpace detects zero by itself
            continue;
        return i;
    }

    return bufSize;
}

//! Пропустить пробелы
inline
size_t skipSpaces( const uint8_t* pBuf, size_t bufSize )
{
    return skipSpaces( (const char*)pBuf, bufSize );
}

//! Пропустить пробельные символы
inline
size_t skipWhitespaces( const char* pBuf )
{
    for( size_t i=0; ; ++i )
    {
        if ( isWhitespace(pBuf[i]) ) // isSpace detects zero by itself
            continue;
        return i;
    }

    //return (size_t)-1;
}

//! Пропустить пробельные символы
inline
size_t skipWhitespaces( const uint8_t* pBuf )
{
    return skipWhitespaces( (const char*)pBuf );
}

//! Пропустить пробельные символы
inline
size_t skipWhitespaces( const char* pBuf, size_t bufSize )
{
    for( size_t i=0; i!=bufSize; ++i )
    {
        if ( isWhitespace(pBuf[i]) )
            continue;
        return i;
    }

    return bufSize;
}

//! Пропустить пробельные символы
inline
size_t skipWhitespaces( const uint8_t* pBuf, size_t bufSize )
{
    return skipWhitespaces( (const char*)pBuf, bufSize );
}


//! Преобразование строки в число, без затей. *pReadedCount - инкрементируется, а не присваивается
inline
int64_t toNumberSimple( const char* pBuf, size_t bufSize, int ss, size_t *pReadedCount = 0 )
{
    //size_t  readedCount = 0;
    int64_t res = 0;
    size_t    i = 0;

    for( ; i!=bufSize && pBuf[i]; ++i )
    {
        int d = toDigit( pBuf[i] );
        if (d<0 || d>=ss)
            break;
        res *= (int64_t)ss;
        res += (int64_t)d;
    }

    if (pReadedCount)
       *pReadedCount += i;

    return res;
}

//! Преобразование строки в число, без затей. *pReadedCount - инкрементируется, а не присваивается
inline
int64_t toNumberSimple( const uint8_t* pBuf, size_t bufSize, int ss, size_t *pReadedCount = 0 )
{
    return toNumberSimple( (const char*)pBuf, bufSize, ss, pReadedCount );
}

//! Преобразование строки в число. *pReadedCount - инкрементируется, а не присваивается
/*! Пропускаются пробельные символы, определяется знак
 */
inline
bool toNumberEx( const char* pBuf, size_t bufSize, int ss, int64_t *pRes, size_t *pReadedCount = 0 )
{
    size_t offs = skipWhitespaces( pBuf, bufSize );
    if (offs>=bufSize)
    {
        if (pReadedCount)
           *pReadedCount += offs;
        return false;
    }

    int64_t neg = 1;
    if (pBuf[offs]=='-')
    {
        ++offs;
        neg = -1;
    }
    else if (pBuf[offs]=='+')
    {
        ++offs;
    }

    if (offs>=bufSize)
    {
        if (pReadedCount)
           *pReadedCount += offs;
        return false;
    }

    size_t tmpReaded = 0;
    int64_t num = toNumberSimple( &pBuf[offs], bufSize-offs, ss, &tmpReaded );
    if (!tmpReaded)
    {
        if (pReadedCount)
           *pReadedCount += offs;
        return false;
    }

    if (pRes)
        *pRes = neg*num;

    if (pReadedCount)
       *pReadedCount += offs+tmpReaded;

    return true;
}

//! \copydoc toNumberEx
inline
bool toNumberEx( const uint8_t* pBuf, size_t bufSize, int ss, int64_t *pRes, size_t *pReadedCount = 0 )
{
    return toNumberEx( (const char*)pBuf, bufSize, ss, pRes, pReadedCount );
}

//! Вычисляет 10 в степени N
inline
int64_t pow10( int powN )
{
    if (powN<0)
        powN = -powN;

    int64_t res = 1;

    for( int i=0; i!=powN; ++i)
    {
        res *= 10;
    }

    return res;
}

//! Умножает/делит на 10 в степени N, в зависимости от знака N
inline
int64_t mulPow10( int64_t val, int powN )
{
    return powN < 0 ? val/pow10(powN) : val*pow10(powN);
}

//! Производит разбор десятичечного числа. *pReadedCount - инкрементируется, а не присваивается
/*! Возвращает число, умноженное на 10 в степени prec.
    Вычитывает всё число целиком, но используется только сколько задано
 */
inline
bool toDecimalFixedPoint( const char* pBuf, size_t bufSize, size_t prec, int64_t *pRes, size_t *pReadedCount = 0 )
{
    size_t orgBufSize = bufSize;
    size_t pos = skipWhitespaces( pBuf, bufSize );
    pBuf    += pos;
    bufSize -= pos;

    if (*pBuf==0 || !bufSize)
        return false;

    bool bNeg = false;

    if (*pBuf=='+' || *pBuf=='-')
    {
        if (*pBuf=='-')
            bNeg = true;

        ++pBuf;
        --bufSize;
    }

    pos = skipWhitespaces( pBuf, bufSize );
    pBuf    += pos;
    bufSize -= pos;

    if (*pBuf==0 || !bufSize)
        return false;

    size_t readedDigits = 0;
    int64_t res = toNumberSimple( pBuf, bufSize, 10, &readedDigits );

    //++readedDigits; // skip dot or comma
    pBuf    += readedDigits;
    bufSize -= readedDigits;

    if (!readedDigits && (*pBuf!='.' && *pBuf!=','))
        return false;

    res = mulPow10( res, (int)prec );

    if (*pBuf==0 || !bufSize || (*pBuf!='.' && *pBuf!=',') )
    {
        if (pRes)
           *pRes = bNeg ? -res : res;
        if (pReadedCount)
           *pReadedCount += orgBufSize - bufSize;
        return true;
    }    

    ++pBuf;
    --bufSize;

    pos = 0;
    int64_t fractional = toNumberSimple( pBuf, bufSize, 10, &pos );
    pBuf    += pos;
    bufSize -= pos;

    readedDigits += pos;
    if (!readedDigits)
    {
        return false;
    }

    int readedPwr10 = (int)pos;
    int takenPwr10  = (int)prec;
    int deltaPwr    = takenPwr10 - readedPwr10;

    fractional = mulPow10( fractional, (int)deltaPwr );
    res += fractional;

    if (pRes)
       *pRes = bNeg ? -res : res;
    if (pReadedCount)
       *pReadedCount += orgBufSize - bufSize;
    return true;
}

//! \copydoc toDecimalFixedPoint 
inline
bool toDecimalFixedPoint( const uint8_t* pBuf, size_t bufSize, size_t prec, int64_t *pRes, size_t *pReadedCount = 0 )
{
    return toDecimalFixedPoint( (const char*)pBuf, bufSize, prec, pRes, pReadedCount );
}



} // namespace parse_utils
} // namespace umba


