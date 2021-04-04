#pragma once

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
#include <cstdint>
#include <climits>
#include <limits>
#include <exception>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <algorithm>
#include <iterator>
#include <string>
#include <algorithm>

//----------------------------------------------------------------------------


/* Все функции/операции в marty::bcd неготовы к неконсистентным данным
   и предназначены только для внутреннего использования

 */

#ifdef min
    #undef min
#endif

#ifdef max
    #undef max
#endif



#ifndef MARTY_BCD_DEFAULT_DIVISION_PRECISION
    #define MARTY_BCD_DEFAULT_DIVISION_PRECISION   18
#endif


//----------------------------------------------------------------------------
namespace marty
{


//----------------------------------------------------------------------------
namespace bcd
{


//----------------------------------------------------------------------------

typedef signed char     decimal_digit_t;
typedef unsigned char   decimal_udigit_t;


#ifndef MARTY_BCD_USE_VECTOR

    typedef std::basic_string<decimal_digit_t> raw_bcd_number_t;

#else

    typedef std::vector<decimal_digit_t> raw_bcd_number_t;

#endif

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
inline
void clearShrink( raw_bcd_number_t &bcdNumber )
{
    bcdNumber.clear();
    bcdNumber.shrink_to_fit();
}
//----------------------------------------------------------------------------
//! Returns precision
/*!
    Нужно еще передавать список символов - допустимых десятичных разделителей
                         список символов - разделителей разрядов
    https://ru.wikipedia.org/wiki/%D0%94%D0%B5%D1%81%D1%8F%D1%82%D0%B8%D1%87%D0%BD%D1%8B%D0%B9_%D1%80%D0%B0%D0%B7%D0%B4%D0%B5%D0%BB%D0%B8%D1%82%D0%B5%D0%BB%D1%8C#:~:text=%D0%A0%D0%B0%D0%B7%D0%B4%D0%B5%D0%BB%D0%B8%D1%82%D0%B5%D0%BB%D1%8C%20%D0%B3%D1%80%D1%83%D0%BF%D0%BF%20%D1%80%D0%B0%D0%B7%D1%80%D1%8F%D0%B4%D0%BE%D0%B2,-%D0%94%D0%BB%D1%8F%20%D1%83%D0%BF%D1%80%D0%BE%D1%89%D0%B5%D0%BD%D0%B8%D1%8F%20%D1%87%D1%82%D0%B5%D0%BD%D0%B8%D1%8F&text=%D0%9A%D0%B0%D0%BA%20%D0%BF%D1%80%D0%B0%D0%B2%D0%B8%D0%BB%D0%BE%2C%20%D0%B3%D1%80%D1%83%D0%BF%D0%BF%D1%8B%20%D1%81%D0%BE%D1%81%D1%82%D0%BE%D1%8F%D1%82%20%D0%B8%D0%B7,%D0%B1%D0%BE%D0%BB%D1%8C%D1%88%D0%B5%20%D1%87%D0%B5%D1%82%D1%8B%D1%80%D1%91%D1%85%20%D0%B8%D0%BB%D0%B8%20%D0%BF%D1%8F%D1%82%D0%B8%20%D1%86%D0%B8%D1%84%D1%80.

    Если в качестве десятичного разделителя используется точка, то разделитель групп разрядов может быть представлен запятой, 
    апострофом или пробелом, а если запятая, — то точкой (например, такая запись прежде использовалась в испанском языке[12], 
    теперь устарела[13]) или пробелом. Таким образом, значение точки и запятой оказывается зависимым от контекста (например, 
    запись 1,546 в английской нотации обозначает одна тысяча пятьсот сорок шесть, а в русской — одна целая пятьсот сорок шесть тысячных).
    Поэтому, чтобы избежать неоднозначности, международные стандарты (ISO 31-0, Международное бюро мер и весов, ИЮПАК) рекомендуют 
    использовать для разделителя групп разрядов только неразрывный пробел (или тонкую шпацию при типографском наборе)
 */

typedef const char* const_char_ptr;

inline
int makeRawBcdNumber( raw_bcd_number_t &bcdNumber
                    , const char *str
                    , std::size_t strSize = (std::size_t)-1
                    , const_char_ptr * pFirstUnknown = 0 //  std::size_t *pProcessedSymbols = 0     // заменить на указатель на указатель на char - начало строки двигается в функции
                    )
{
    if (strSize==(std::size_t)-1)
        strSize = std::strlen(str);

    bcdNumber.clear();

    int precision = 0;

    std::size_t processedSymbols = 0;

    bool processingFranctionalPart = false;

    bool hasDecimalDot = false;
    for(std::size_t i=0; i!=strSize; ++i)
    {
        if ( (str[i] == '.') || (str[i] == ',') )
        {
            hasDecimalDot = true;
            break;
        }
    }

    if (hasDecimalDot)
    {
        while(strSize!=0 && str[strSize-1]=='0') // "remove" trailing zeros
            --strSize;
    }

    while( strSize>1 && *str=='0') // "remove" leading zeros, but keep one
        { --strSize; ++str; }

    bcdNumber.reserve(strSize+1);


    //for(; *str; ++str, ++processedSymbols /* , ++precision */  )
    std::size_t i = 0;

    for( ; i!=strSize; ++i)
    {
        if ( (str[i] == '.') || (str[i] == ',') )
        {
            if (processingFranctionalPart)
            {
                // Already processing franctional part
                if (pFirstUnknown)
                   *pFirstUnknown = &str[i];
                std::reverse(bcdNumber.begin(), bcdNumber.end());
                return precision;
            }

            processingFranctionalPart = true;

            if (!processedSymbols)
            {
                bcdNumber.push_back(0);
            }

            ++processedSymbols;
            continue;
        }

        if ( (str[i] < '0') || (str[i] > '9') )
        {
            if (pFirstUnknown)
               *pFirstUnknown = &str[i];

            std::reverse(bcdNumber.begin(), bcdNumber.end());
            return precision;
        }

        decimal_digit_t d = (decimal_digit_t)(str[i] - '0');
        ++processedSymbols;

        bcdNumber.push_back(d);

        if (processingFranctionalPart)
            ++precision;
    }

    if (pFirstUnknown)
       *pFirstUnknown = &str[i];

    std::reverse(bcdNumber.begin(), bcdNumber.end());
    return precision;

}

//----------------------------------------------------------------------------
template<typename UnsignedIntegerType>
inline
int makeRawBcdNumberFromUnsigned( raw_bcd_number_t &bcdNumber
                                , UnsignedIntegerType u
                                )
{
    bcdNumber.clear();

    while(u)
    {
        bcdNumber.push_back( u%10 );
        u /= 10;
    }

    if (bcdNumber.empty())
       bcdNumber.push_back( 0 );

    return 0;

}

//----------------------------------------------------------------------------
//! Обрезаем точность, если возможно (в хвосте есть нули) до десятичной точки
//!!!
inline
int reducePrecision( raw_bcd_number_t &bcdNumber, int precision )
{
    int i = 0, size = (int)bcdNumber.size();

    if (precision<0)
        return precision;


    for(; i!=size && i!=precision; ++i)
    {
        if (bcdNumber[i]!=0)
            break;
    }

    std::size_t numberOfPositionsToReduce = i;

    if (numberOfPositionsToReduce > precision)
        numberOfPositionsToReduce = precision;

    //std::size_t numberOfPositionsToReduce = precision - i;

    if (!numberOfPositionsToReduce)
        return precision;

    raw_bcd_number_t::iterator eraseEnd = bcdNumber.begin();
    std::advance(eraseEnd, numberOfPositionsToReduce);

    bcdNumber.erase( bcdNumber.begin(), eraseEnd );

    return precision - numberOfPositionsToReduce;
}

//----------------------------------------------------------------------------
//! Обрезаем точность, если возможно (в хвосте есть нули), даже если пересекаем позицию десятичной точки
//!!!
inline
int reducePrecisionFull( raw_bcd_number_t &bcdNumber, int precision )
{
    if (bcdNumber.size()<2)
        return precision;

    int i = 0, size = (int)bcdNumber.size();

    for(; i!=(size-1); ++i)
    {
        if (bcdNumber[i]!=0)
            break;
    }

    if (!i)
        return precision;

    std::size_t numberOfPositionsToReduce = i;

    raw_bcd_number_t::iterator eraseEnd = bcdNumber.begin();
    std::advance(eraseEnd, numberOfPositionsToReduce);

    bcdNumber.erase( bcdNumber.begin(), eraseEnd );

    return precision - numberOfPositionsToReduce;
}

//----------------------------------------------------------------------------
//! Обрезаем ведущие незначащие нули
//!!!
inline
int reduceLeadingZeros( raw_bcd_number_t &bcdNumber, int precision )
{
    /*
    bool  bEmpty    =  bcdNumber.empty();
    bool nbEmpty    = !bcdNumber.empty();
    bool backIsZero = 
    */
    while( (bcdNumber.size()>0) && (bcdNumber.back()==0) && (bcdNumber.size() > (precision+1)) )
    {
        bcdNumber.pop_back();
    }

    return precision;
}

//----------------------------------------------------------------------------
//! Обрезаем ведущие нули, включая те, которые после точки
//!!!
inline
int reduceLeadingZerosFull( raw_bcd_number_t &bcdNumber, int precision )
{
    precision = reduceLeadingZeros( bcdNumber, precision );

    while( (bcdNumber.size()>0) && (bcdNumber.back()==0) )
    {
        bcdNumber.pop_back();
    }

    return precision;
}

//----------------------------------------------------------------------------
//! Расширяем точность до заданной
inline
int extendPrecision( raw_bcd_number_t &bcdNumber, int curPrecision, int newPrecision )
{
    if (newPrecision<0) // Так низя
        newPrecision = 0;

    if (newPrecision<=curPrecision) // Нет расширения
        return curPrecision;

    int deltaPrecision = newPrecision - curPrecision;

    // void insert( iterator pos, size_type count, const T& value );
    bcdNumber.insert( bcdNumber.begin(), (raw_bcd_number_t::size_type)deltaPrecision, (raw_bcd_number_t::value_type)0 );

    return newPrecision;

}

//----------------------------------------------------------------------------
//! Расширяем перед точкой
//!!!
inline
int extendLeadings( raw_bcd_number_t &bcdNumber, int precision, int requestedLeadings )
{
    int bcdSize = (int)bcdNumber.size();

    if (bcdSize<precision)
        return precision;

    std::size_t curLeadings = bcdSize - precision;

    if (requestedLeadings<curLeadings)
        return precision;

    int leadingsToAdd = requestedLeadings - curLeadings;

    if (!leadingsToAdd)
        return precision;

    bcdNumber.insert( bcdNumber.end(), (raw_bcd_number_t::size_type)leadingsToAdd, (raw_bcd_number_t::value_type)0 );

    return precision;
}

//----------------------------------------------------------------------------
//! Проверка на ноль
inline
bool checkForZero( const raw_bcd_number_t &bcdNumber )
{
    if (bcdNumber.empty())
        return true;

    for( auto it=bcdNumber.begin(); it!=bcdNumber.end(); ++it )
    {
        if (*it!=0)
            return false;
    }

    return true;
}

//----------------------------------------------------------------------------
inline
int getMaxPrecision( int precision1, int precision2 )
{
    int res = (precision1 > precision2) ? precision1 : precision2;
    if (res<0)
        res = 0;
    return res;
}

//----------------------------------------------------------------------------
inline
int getMinPrecision( int precision1, int precision2 )
{
    int res = (precision1 < precision2) ? precision1 : precision2;
    if (res<0)
        res = 0;
    return res;
}

//----------------------------------------------------------------------------
inline
int getIntegerPartSize( const raw_bcd_number_t &bcdNumber, int precision )
{
    int res = (int)bcdNumber.size() - precision;
    if (res<0)
        res = 1;
    return res; // (int)bcdNumber.size() - precision;
}

//----------------------------------------------------------------------------
inline
int getMaxIntegerPartSize( int ip1
                         , int ip2
                         )
{
    return (ip1 > ip2) ? ip1 : ip2;
}

//----------------------------------------------------------------------------
inline
int getMaxIntegerPartSize( const raw_bcd_number_t &bcdNumber1, int precision1
                         , const raw_bcd_number_t &bcdNumber2, int precision2
                         )
{
    return getMaxIntegerPartSize( getIntegerPartSize( bcdNumber1, precision1 )
                                , getIntegerPartSize( bcdNumber2, precision2 )
                                );
}

//----------------------------------------------------------------------------
inline
int getMinIntegerPartSize( int ip1
                         , int ip2
                         )
{
    return (ip1 < ip2) ? ip1 : ip2;
}

//----------------------------------------------------------------------------
inline
int getMinIntegerPartSize( const raw_bcd_number_t &bcdNumber1, int precision1
                         , const raw_bcd_number_t &bcdNumber2, int precision2
                         )
{
    return getMinIntegerPartSize( getIntegerPartSize( bcdNumber1, precision1 )
                                , getIntegerPartSize( bcdNumber2, precision2 )
                                );
}

//----------------------------------------------------------------------------
//! Lookup for Most/Least Significant Digit (MSD/LSD). Младшие цифры идут с начала.
inline
raw_bcd_number_t::size_type getMsdIndex( const raw_bcd_number_t &bcdNumber )
{
    raw_bcd_number_t::size_type i = bcdNumber.size();

    for( ; i!=0; --i)
    {
        if (bcdNumber[i-1]!=0)
            return i-1;
    }

    if (!bcdNumber.empty())
        return 0;

    return (raw_bcd_number_t::size_type)-1;

}

//----------------------------------------------------------------------------
//! Lookup for Most/Least Significant Digit (MSD/LSD). Младшие цифры идут с начала.
inline
raw_bcd_number_t::size_type getLsdIndex( const raw_bcd_number_t &bcdNumber )
{
    raw_bcd_number_t::size_type i = 0, size = bcdNumber.size();
    for( ; i!=size; ++i)
    {
        if (bcdNumber[i]!=0)
            return i;
    }

    if (!bcdNumber.empty())
        return 0;

    return (raw_bcd_number_t::size_type)-1;
}

//----------------------------------------------------------------------------
#define MARTY_BCD_DECLARE_PRECISION_VIRTUAL_ADJUSTMENT_VARS()                                  \
                    int ipSize1      = getIntegerPartSize( bcdNumber1, precision1 );           \
                    int ipSize2      = getIntegerPartSize( bcdNumber2, precision2 );           \
                                                                                               \
                    int maxIpSize    = getMaxIntegerPartSize( ipSize1, ipSize2 );              \
                    /*std::size_t minIpSize    = getMinIntegerPartSize( ipSize1, ipSize2 );*/  \
                                                                                               \
                    int maxPrecision = getMaxPrecision( precision1, precision2 );              \
                    int minPrecision = getMinPrecision( precision1, precision2 );              \
                    int dtPrecision  = maxPrecision - minPrecision;                            \
                                                                                               \
                    int offset1      = maxPrecision - precision1;                              \
                    int offset2      = maxPrecision - precision2;                              \
                                                                                               \
                    int totalSize    = maxIpSize; /* virtual size */                           \
                    if (maxPrecision>0)                                                        \
                        totalSize += maxPrecision;


#define MARTY_BCD_DECLARE_PRECISION_VIRTUAL_ADJUSTMENT_VARS_V2( bcdNumber1, precision1, bcdNumber2, precision2 )  \
                int decOrderMin1 = -precision1;                                                                   \
                int decOrderMax1 = decOrderMin1 + (int)bcdNumber1.size();                                         \
                                                                                                                  \
                int decOrderMin2 = -precision2;                                                                   \
                int decOrderMax2 = decOrderMin2 + (int)bcdNumber2.size();                                         \
                                                                                                                  \
                int decOrderMin = std::min( decOrderMin1, decOrderMin2 );                                         \
                int decOrderMax = std::max( decOrderMax1, decOrderMax2 );                                         \
                                                                                                                  \
                int maxPrecision = std::max( precision1, precision2 );                                            \
                                                                                                                  \
                int decOrderDelta = decOrderMax - decOrderMin;


#define MARTY_BCD_PRECISION_GET_DIGITS_BY_VIRTUAL_ADJUSTMENT_VARS( decOrderValue, bcdNumber1, bcdNumber2 )        \
                                                                                                                  \
                int idx1 = (decOrderValue) - decOrderMin1;                                                        \
                int idx2 = (decOrderValue) - decOrderMin2;                                                        \
                                                                                                                  \
                decimal_digit_t d1 = 0;                                                                           \
                decimal_digit_t d2 = 0;                                                                           \
                                                                                                                  \
                if (idx1>=0 && idx1<(int)bcdNumber1.size())                                                       \
                    d1 = bcdNumber1[idx1];                                                                        \
                                                                                                                  \
                if (idx2>=0 && idx2<(int)bcdNumber2.size())                                                       \
                    d2 = bcdNumber2[idx2];

//----------------------------------------------------------------------------
inline
int getDecimalOrderByIndex( raw_bcd_number_t::size_type idx, const raw_bcd_number_t &bcdNumber, int precision )
{
    int order = 0;

    if (idx!=(raw_bcd_number_t::size_type)-1)
        order = ((int)idx) - precision;

    return order;
}

//----------------------------------------------------------------------------
inline
int truncatePrecision( raw_bcd_number_t &bcdNumber, int precision, int newPrecision, int *pLastTruncatedDigit = 0, bool *pAllTruncatedAreZeros = 0 )
{
    if (newPrecision<0)
        newPrecision = 0; // truncution of integer part is not allowed

    if (newPrecision>=precision)
    {
        if (pLastTruncatedDigit)
           *pLastTruncatedDigit = 0;

        if (pAllTruncatedAreZeros)
           *pAllTruncatedAreZeros = true; // allTruncatedAreZeros;

        return precision; // newPrecision; // Nothing to do
    }
    // Realy need truncation

    int requestedDecOrder = -newPrecision;

    int lastTruncatedDigit = 0;
    bool allTruncatedAreZeros = true;

    raw_bcd_number_t::size_type digitIndex = 0, bcdSize = bcdNumber.size();

    for(; digitIndex!=bcdSize; ++digitIndex)
    {
        int decOrder = getDecimalOrderByIndex( digitIndex, bcdNumber, precision );

        if (decOrder>=requestedDecOrder)
        {
            if (pLastTruncatedDigit)
               *pLastTruncatedDigit = lastTruncatedDigit;

            if (pAllTruncatedAreZeros)
               *pAllTruncatedAreZeros = allTruncatedAreZeros;

            raw_bcd_number_t::iterator eraseEnd = bcdNumber.begin();
            std::advance(eraseEnd, digitIndex);

            for(raw_bcd_number_t::iterator it=bcdNumber.begin(); it!=eraseEnd; ++it )
            {
                if (*it!=0)
                    allTruncatedAreZeros = false;
            }

            bcdNumber.erase( bcdNumber.begin(), eraseEnd );

            return newPrecision;
        }

        lastTruncatedDigit = bcdNumber[digitIndex];
        if (bcdNumber[digitIndex]!=0)
            allTruncatedAreZeros = false;

    }

    if (pLastTruncatedDigit)
       *pLastTruncatedDigit = 0; // lastTruncatedDigit;

    if (pAllTruncatedAreZeros)
       *pAllTruncatedAreZeros = true; // allTruncatedAreZeros;
       
    return precision;
}

//----------------------------------------------------------------------------
inline
int compareRaws( const raw_bcd_number_t &bcdNumber1, int precision1
               , const raw_bcd_number_t &bcdNumber2, int precision2
               )
{
    // Как быть, вот в чем вопрос?
    //
    // 1) Можно уравнять
    //    а) в обоих bcdNumbers precisions (расширением снизу/at front),
    //    б) целую часть (расширением сверху/at back)
    //    Тогда на каждом шаге не будет лишних проверок и сама операция будет произведена быстро.
    //    Но будут переаллокации вектора(ов) BCD, и их же перемещения.
    // 2) Можно ничего не переаллоцировать/перемещать, но на каждом 
    //    шаге будет смещение и проверки индексов.
    //    Зато не придётся лазать в кучу

    MARTY_BCD_DECLARE_PRECISION_VIRTUAL_ADJUSTMENT_VARS_V2( bcdNumber1, precision1, bcdNumber2, precision2 )


    // Сравнение начинаем со старших разрядов

    for( int decOrder=decOrderMax+1; decOrder!=decOrderMin; --decOrder )
    {

        //MARTY_BCD_PRECISION_GET_DIGITS_BY_VIRTUAL_ADJUSTMENT_VARS( decOrder-1, bcdNumber1, bcdNumber2 )
        MARTY_BCD_PRECISION_GET_DIGITS_BY_VIRTUAL_ADJUSTMENT_VARS( decOrder-1, bcdNumber1, bcdNumber2 )

        if (d1<d2)
            return -1;

        if (d1>d2)
            return  1;
    }

    return 0;
}

//----------------------------------------------------------------------------
//! Форматирование "сырого" BCD, форматируется в строку с полной точностью, без какого-либо усечения/округления
inline
const char* formatRawBcdNumber( const raw_bcd_number_t &bcdNumber, int precision, char *pBuf, std::size_t bufSize, char sep = '.' )
{
    if (bufSize < 4 )
        throw std::runtime_error("marty::bcd::formatRawBcdNumber: bufSize is not enough");

    // bufSize must be enough for 0.0\0

    --bufSize; // Reserve space for termination zero

    std::size_t bufPos = 0;

    
    //int maxOrder = getDecimalOrderByIndex( getMsdIndex(bcdNumber), bcdNumber, precision );

    int decOrderMin = -precision;
    if (decOrderMin>0)
        decOrderMin = 0; // Не гоже отрезать значащие нули справа от недробной части


    // int decOrderMax = decOrderMin + (int)bcdNumber.size();

    // Тут мы точно находим максимальный значащий разряд, опуская возможные ведущие нули
    // При операциях на них плевать, тем более что нули во всех операциях оптимизированы
    // А при выводе ведущие нули, хз откуда выплывшие, нам не нужны
    int decOrderMax = getDecimalOrderByIndex( getMsdIndex(bcdNumber), bcdNumber, precision ); 

    // Но нам нужны ведущие нули, если целая часть числа равна нулю
    if (decOrderMax<0)
        decOrderMax = 0;

    //int maxPrecision = std::max( precision1, precision2 );

    int decOrderDelta = decOrderMax - decOrderMin;

    // Форматирование, как и сравение, начинаем со старших разрядов

    for( int decOrder=decOrderMax+1; decOrder!=decOrderMin; --decOrder )
    {
        // Я просто выдрал код из макроса, который для двух чисел, decOrderValue там - параметр макроса.
        // Чтобы было минимум расхождений с макросом.
        // А у нас тут только одно число, и лишние дествия хоть и копеечны, но зачем?
        // Теоретически, макросы для двух чисел можно переделать на макросы для одного числа, 
        // и использовать дважды, но чёй-то лениво, и особо уже и незачем.
        // Форматирование - единственная операция, где участвует одно число.
        // Сравнение, сложение, вычитание, умножение и деление - там всё парнокопытно. И всё же отлажено и работает.
        // Форматирование было написано прежде всего, но когда я пытался использовать код из ф-ии форматирования
        // в арифм. операциях баги полезли сразу. А форматирование до последнего делало вид, что работает правильно.

        const int decOrderValue = decOrder-1; 

        int idx = (decOrderValue) - (-precision); // decOrderMin;

        decimal_digit_t d = 0;

        if (idx>=0 && idx<(int)bcdNumber.size())
            d = bcdNumber[idx];

        if (decOrderValue==-1 && bufPos!=0) // Добрались до первой цифры после точки
        {
            if (bufPos<bufSize)
                pBuf[bufPos++] = sep;
            //*pBuf++ = sep;
        }

        if (bufPos<bufSize)
            pBuf[bufPos++] = d + '0';
    }

    if (!bufPos) // место есть, спасибо --bufSize;
    {
        pBuf[bufPos++] = '0';
    }

    pBuf[bufPos] = 0; // место есть, спасибо --bufSize;

    return pBuf;


}

//----------------------------------------------------------------------------
inline
std::string formatRawBcdNumber( const raw_bcd_number_t &bcdNumber, int precision, char sep = '.' )
{
    char buf[1024];
    return formatRawBcdNumber( bcdNumber, precision, &buf[0], sizeof(buf), sep );
}

//----------------------------------------------------------------------------
inline
int getLowestDigit( const raw_bcd_number_t &bcdNumber, int precision )
{
    const int decOrderMin   = -precision;
    const int decOrderValue = decOrderMin; // decOrder-1; 

    int idx = (decOrderValue) - (-precision); // decOrderMin;

    decimal_digit_t d = 0;

    if (idx>=0 && idx<(int)bcdNumber.size())
        d = bcdNumber[idx];

    return d;
}

//----------------------------------------------------------------------------
//! Конвертация в целое
inline
std::uint64_t rawToInt( const raw_bcd_number_t &bcdNumber, int precision )
{

    int decOrderMin = -precision;
    if (decOrderMin>0)
        decOrderMin = 0; // Не гоже отрезать значащие нули справа от недробной части


    int decOrderMax = getDecimalOrderByIndex( getMsdIndex(bcdNumber), bcdNumber, precision ); 

    // Но нам нужны ведущие нули, если целая часть числа равна нулю
    if (decOrderMax<0)
        decOrderMax = 0;

    int decOrderDelta = decOrderMax - decOrderMin;

    std::uint64_t res = 0;

    for( int decOrder=decOrderMax+1; decOrder!=decOrderMin; --decOrder )
    {
        const int decOrderValue = decOrder-1; 

        int idx = (decOrderValue) - (-precision); // decOrderMin;

        decimal_digit_t d = 0;

        if (idx>=0 && idx<(int)bcdNumber.size())
            d = bcdNumber[idx];

        if (decOrderValue==-1) // Добрались до первой цифры после точки
        {
            return res;
        }

        res *= 10;
        res += (unsigned)d;

    }

    return res;

}

//----------------------------------------------------------------------------
//! Конвертация в целое
inline
double rawToDouble( const raw_bcd_number_t &bcdNumber, int precision )
{

    int decOrderMin = -precision;
    if (decOrderMin>0)
        decOrderMin = 0; // Не гоже отрезать значащие нули справа от недробной части


    int decOrderMax = getDecimalOrderByIndex( getMsdIndex(bcdNumber), bcdNumber, precision ); 

    // Но нам нужны ведущие нули, если целая часть числа равна нулю
    if (decOrderMax<0)
        decOrderMax = 0;

    int decOrderDelta = decOrderMax - decOrderMin;

    double res = 0;
    double devider = 1;

    for( int decOrder=decOrderMax+1; decOrder!=decOrderMin; --decOrder )
    {
        const int decOrderValue = decOrder-1; 

        int idx = (decOrderValue) - (-precision); // decOrderMin;

        decimal_digit_t d = 0;

        if (idx>=0 && idx<(int)bcdNumber.size())
            d = bcdNumber[idx];

        if (decOrderValue<=-1) // Добрались до цифр после точки
        {
            devider *= 10;
        }

        res *= 10;
        res += (unsigned)d;

    }

    return res/devider;

}

//----------------------------------------------------------------------------
inline
decimal_digit_t bcdCorrectOverflow( decimal_digit_t &d )
{
    // d must be >= 0
    
    decimal_digit_t res = d / 10;
    d %= 10;
    return res;
}

//----------------------------------------------------------------------------
inline
decimal_digit_t bcdCorrectCarry( decimal_digit_t &d )
{
    if (d>=0)
        return 0;

    d = -d;

    d = ((decimal_digit_t)10) - d;
    return 1;
}

//----------------------------------------------------------------------------
//! Сложение "сырых" BCD чисел с "плавающей" точкой
inline
int rawAddition( raw_bcd_number_t &bcdRes
               , const raw_bcd_number_t &bcdNumber1, int precision1
               , const raw_bcd_number_t &bcdNumber2, int precision2
               )
{

    MARTY_BCD_DECLARE_PRECISION_VIRTUAL_ADJUSTMENT_VARS_V2( bcdNumber1, precision1, bcdNumber2, precision2 )

    bcdRes.clear();
    bcdRes.reserve(decOrderDelta+1);

    decimal_digit_t dPrev = 0;

    for( int decOrder=decOrderMin; decOrder!=(decOrderMax+1); ++decOrder )
    {
        MARTY_BCD_PRECISION_GET_DIGITS_BY_VIRTUAL_ADJUSTMENT_VARS( decOrder, bcdNumber1, bcdNumber2 )

        decimal_digit_t dCur = dPrev + d1 + d2;
        dPrev = bcdCorrectOverflow(dCur);

        bcdRes.push_back(dCur);

    }

    if (dPrev)
        bcdRes.push_back(dPrev);

    int resultPrecision = maxPrecision;

    resultPrecision = reduceLeadingZeros( bcdRes, resultPrecision );
    resultPrecision = reducePrecision   ( bcdRes, resultPrecision );

    return resultPrecision;
}

//----------------------------------------------------------------------------
//! Вычитание "сырых" BCD чисел с "плавающей" точкой. Уменьшаемое должно быть больше или равно вычитаемому
inline
int rawSubtraction( raw_bcd_number_t &bcdRes
                  , const raw_bcd_number_t &bcdNumber1, int precision1
                  , const raw_bcd_number_t &bcdNumber2, int precision2
                  )
{   /*
    MARTY_BCD_DECLARE_PRECISION_VIRTUAL_ADJUSTMENT_VARS();

    bcdRes.clear();
    bcdRes.reserve(totalSize+1);

    decimal_digit_t dPrev = 0;

    for( int idxFromBegin=0; idxFromBegin!=totalSize; ++idxFromBegin)
    {
        int idx  = (int)idxFromBegin;

        int idx1 = idx - (int)maxPrecision;
        int idx2 = idx - (int)maxPrecision;

        idx1 += (int)precision1;
        idx2 += (int)precision2;

        decimal_digit_t d1 = 0;
        decimal_digit_t d2 = 0;

        if (idx1>=0 && idx1<(int)bcdNumber1.size())
            d1 = bcdNumber1[idx1];

        if (idx2>=0 && idx2<(int)bcdNumber2.size())
            d2 = bcdNumber2[idx2];
        */

    MARTY_BCD_DECLARE_PRECISION_VIRTUAL_ADJUSTMENT_VARS_V2( bcdNumber1, precision1, bcdNumber2, precision2 )

    bcdRes.clear();
    bcdRes.reserve(decOrderDelta+1);

    decimal_digit_t dPrev = 0;

    for( int decOrder=decOrderMin; decOrder!=(decOrderMax+1); ++decOrder )
    {
        MARTY_BCD_PRECISION_GET_DIGITS_BY_VIRTUAL_ADJUSTMENT_VARS( decOrder, bcdNumber1, bcdNumber2 )

        decimal_digit_t dCur = d1 - dPrev; // - d2;
        dPrev = bcdCorrectCarry(dCur);

        dCur = dCur - d2;
        dPrev += bcdCorrectCarry(dCur);

        bcdRes.push_back(dCur);

    }

    if (dPrev)
        bcdRes.push_back(dPrev);

    int resultPrecision = maxPrecision;

    resultPrecision = reduceLeadingZeros( bcdRes, resultPrecision );
    resultPrecision = reducePrecision   ( bcdRes, resultPrecision );

    return resultPrecision;
}

//----------------------------------------------------------------------------
inline
raw_bcd_number_t rawBcdMakeZeroPrecisionFromNegative( const raw_bcd_number_t &bcdNumber, int precision )
{
    if (precision>=0)
        return bcdNumber;

    // precision < 0 here

    precision = -precision;

    raw_bcd_number_t res;
    res.reserve( bcdNumber.size() + (std::size_t)precision );

    res.insert( res.end(), (std::size_t)precision, 0 ); // extends with zeros on lowest positions
    res.insert( res.end(), bcdNumber.begin(), bcdNumber.end() );

    return res; // precision is zero here
}

//----------------------------------------------------------------------------
//! Умножение "сырых" BCD чисел с "плавающей" точкой.
/*! Тупое умножение в столбик.
    Небольшая оптимизация - нулевые частичные суммы не вычисляем.
    Чем больше нулей в числе - тем быстрее производится умножение.
 */
inline
int rawMultiplication( raw_bcd_number_t &multRes
                  , const raw_bcd_number_t &bcdNumberArg1, int precision1
                  , const raw_bcd_number_t &bcdNumberArg2, int precision2
                  )
{
    //MARTY_BCD_DECLARE_PRECISION_VIRTUAL_ADJUSTMENT_VARS();

    multRes.clear();

    typedef const raw_bcd_number_t*  const_raw_bcd_number_ptr_t;

    const_raw_bcd_number_ptr_t ptrBcdNumber1 = &bcdNumberArg1;
    const_raw_bcd_number_ptr_t ptrBcdNumber2 = &bcdNumberArg2;

    raw_bcd_number_t correctedBcdNumber1;
    raw_bcd_number_t correctedBcdNumber2;

    if (precision1<0)
    {
        correctedBcdNumber1 = rawBcdMakeZeroPrecisionFromNegative( bcdNumberArg1, precision1 );
        precision1          = 0;
        ptrBcdNumber1       = &correctedBcdNumber1;
    }

    if (precision2<0)
    {
        correctedBcdNumber2 = rawBcdMakeZeroPrecisionFromNegative( bcdNumberArg2, precision2 );
        precision2          = 0;
        ptrBcdNumber2       = &correctedBcdNumber2;
    }

    const raw_bcd_number_t &bcdNumber1 = *ptrBcdNumber1;
    const raw_bcd_number_t &bcdNumber2 = *ptrBcdNumber2;


    //raw_bcd_number_t multRes;
    multRes.reserve( bcdNumber1.size() + bcdNumber2.size() );

    raw_bcd_number_t partialMult;
    partialMult.reserve( bcdNumber1.size() + bcdNumber2.size() );

    raw_bcd_number_t tmpBcd;
    tmpBcd.reserve(multRes.size());

    decimal_digit_t digitHigh = 0;
    decimal_digit_t digitCur  = 0;


    for( std::size_t i=0; i!=bcdNumber1.size(); ++i)
    {
        auto d1   = bcdNumber1[i];
        if (d1==0)
            continue;

        for( std::size_t j=0; j!=bcdNumber2.size(); ++j)
        {
            auto d2   = bcdNumber2[j];
            if (d2==0)
                continue;

            digitCur  = d1*d2;

            partialMult.clear();
            partialMult.insert( partialMult.end(), i+j, 0 ); // extends with zeros on lowest positions

            digitHigh = bcdCorrectOverflow(digitCur);

            partialMult.insert( partialMult.end(), 1, digitCur  );
            partialMult.insert( partialMult.end(), 1, digitHigh );

            rawAddition( tmpBcd, multRes, 0, partialMult, 0 );

            tmpBcd.swap(multRes);
        }
    }

    int precisionRes = precision1+precision2;
    return reducePrecision( multRes, precisionRes );
}

//----------------------------------------------------------------------------
inline
bool rawDivisionCheckContinueCondition( int dividendPrecision, int divisorPrecision
                                      , int  deltaCmp
                                      , bool relativeDelta
                                      )
{
    int delta = dividendPrecision - divisorPrecision;

    if (relativeDelta)
    {
        if (delta < deltaCmp)
            return true; // continue division
        return false;
    }
    else
    {
        // absolute precision required
        // Hm-m-m
        throw std::runtime_error("rawDivisionCheckContinueCondition: absolute precision not implemented");
    }

    return false;

}

//----------------------------------------------------------------------------
//! Деление "сырых" BCD чисел с "плавающей" точкой.
inline
int rawDivision( raw_bcd_number_t &quotient
               , raw_bcd_number_t dividend, int dividendPrecision
               , raw_bcd_number_t divisor , int divisorPrecision
               , int  deltaCmp = MARTY_BCD_DEFAULT_DIVISION_PRECISION
               , bool relativeDelta = true
               )
{
    // Удаляем все нули справа от значащих цифр, до точки или после - не важно
    dividendPrecision = reducePrecisionFull( dividend, dividendPrecision );
    divisorPrecision  = reducePrecisionFull( divisor , divisorPrecision  );
    // Или, таки важно? А то чёй-то не так работает
    // dividendPrecision = reducePrecision( dividend, dividendPrecision );
    // divisorPrecision  = reducePrecision( divisor , divisorPrecision  );

    dividendPrecision = reduceLeadingZerosFull( dividend, dividendPrecision );
    divisorPrecision  = reduceLeadingZerosFull( divisor , divisorPrecision  );

    // Нужно выровнять размеры
    if (dividend.size() < divisor.size())
    {
        //std::cout<<"NNN1" << std::endl;
        std::size_t delta = divisor.size() - dividend.size();
        dividend.insert( dividend.begin(), delta, 0 );
        dividendPrecision += (int)delta;
    }
    else if (dividend.size() > divisor.size())
    {
        //std::cout<<"NNN2" << std::endl;
        std::size_t delta = dividend.size() - divisor.size();
        divisor.insert( divisor.begin(), delta, 0 );
        divisorPrecision += (int)delta;
    }


    // Не размеры надо выровнять, а порядок старшей значащей цифры (вернее, положение в векторе, 
    // так как порядок далее везде берём нулевой при вычислениях)

    /*
    int dividendMsdOrder = getDecimalOrderByIndex( getMsdIndex(dividend), dividend, dividendPrecision );
    int divisorMsdOrder  = getDecimalOrderByIndex( getMsdIndex(divisor ), divisor , divisorPrecision  );

    if (dividendMsdOrder > divisorMsdOrder)
    {
        std::cout<<"NNN1" << std::endl;
        int delta = dividendMsdOrder - divisorMsdOrder;
        divisor.insert( divisor.begin(), delta, 0 );
        divisorPrecision += (int)delta;
    }
    if (dividendMsdOrder < divisorMsdOrder)
    {
        std::cout<<"NNN2" << std::endl;
        int delta = divisorMsdOrder - dividendMsdOrder;
        dividend.insert( dividend.begin(), delta, 0 );
        dividendPrecision += (int)delta;
    }
    */

//int getDecimalOrderByIndex( raw_bcd_number_t::size_type idx, const raw_bcd_number_t &bcdNumber, int precision )
//raw_bcd_number_t::size_type getMsdIndex( const raw_bcd_number_t &bcdNumber )



    //int resultPrecision = dividendPrecision - divisorPrecision;

    quotient.clear();

    // Делимое должно быть больше делителя, поэтому умножаем на 10 и инкрементируем показатель степени
    // Но сравниваем так, как-будто степень у обоих чисел одинаковая - нулевая
    //while( compareRaws( dividend, dividendPrecision, divisor , divisorPrecision ) < 0 )
    /*
    while( compareRaws( dividend,                 0, divisor ,                0 ) < 0 )
    {
        dividend.insert( dividend.begin(), 1, 0 );
        ++dividendPrecision;
    }
    */

    //quotient.reserve(decOrderDelta+1);

    raw_bcd_number_t tmp;

    quotient.insert( quotient.begin(), 1, 0 );

    //while( (dividendPrecision - divisorPrecision) < 18 )
    while( rawDivisionCheckContinueCondition( dividendPrecision, divisorPrecision, deltaCmp, relativeDelta ) )
    {
        if (checkForZero( dividend ) )
            break;

        while( compareRaws( dividend,                 0, divisor ,                0 ) < 0 )
        {
            dividend.insert( dividend.begin(), 1, 0 );
            ++dividendPrecision;
            quotient.insert( quotient.begin(), 1, 0 );
        }

        while( compareRaws( dividend, 0, divisor , 0 ) >= 0 )
        {
            rawSubtraction( tmp, dividend, 0, divisor , 0 );
            tmp.swap(dividend);
            ++quotient[0];
        }

    }
    
    //return 0;

    int 
    resultPrecision = dividendPrecision - divisorPrecision;
    resultPrecision = reducePrecisionFull( quotient, resultPrecision );
    resultPrecision = reduceLeadingZerosFull( quotient, resultPrecision );

    quotient.shrink_to_fit();

    return resultPrecision;
}


/* Шаблон перебора цифр от старших к младшим - сравнение, возможно - деление

inline
int tplFromMsdToLsd( const raw_bcd_number_t &bcdNumber1, int precision1
                   , const raw_bcd_number_t &bcdNumber2, int precision2
                   )
{
    MARTY_BCD_DECLARE_PRECISION_VIRTUAL_ADJUSTMENT_VARS();

    for( int idxFromBegin=0; idxFromBegin!=totalSize; ++idxFromBegin)
    {
        int idx  = (int)idxFromBegin;

        int idx1 = totalSize - idx - 1;
        int idx2 = totalSize - idx - 1;

        idx1 -= (int)maxPrecision - (int)precision1;
        idx2 -= (int)maxPrecision - (int)precision2;


        decimal_digit_t d1 = 0;
        decimal_digit_t d2 = 0;

        if (idx1>=0 && idx1<(int)bcdNumber1.size())
            d1 = bcdNumber1[idx1];

        if (idx2>=0 && idx2<(int)bcdNumber2.size())
            d2 = bcdNumber2[idx2];

        // Do something with digits here

    }

    return 0;
}

*/


/* Шаблон перебора цифр от младших к старшим - сложение, вычитание, умножение

inline
int templateForOperation( const raw_bcd_number_t &bcdNumber1, int precision1
                        , const raw_bcd_number_t &bcdNumber2, int precision2
                        )
{
    MARTY_BCD_DECLARE_PRECISION_VIRTUAL_ADJUSTMENT_VARS();

    for( int idxFromBegin=0; idxFromBegin!=totalSize; ++idxFromBegin)
    {
        int idx  = (int)idxFromBegin;

        int idx1 = idx - (int)maxPrecision;
        int idx2 = idx - (int)maxPrecision;

        idx1 += (int)precision1;
        idx2 += (int)precision2;

        decimal_digit_t d1 = 0;
        decimal_digit_t d2 = 0;

        if (idx1>=0 && idx1<(int)bcdNumber1.size())
            d1 = bcdNumber1[idx1];

        if (idx2>=0 && idx2<(int)bcdNumber2.size())
            d2 = bcdNumber2[idx2];

        std::cout<<(unsigned)(d1);

    }

    return 0;
}
*/




/* Что ещё необходимо:

     Идея generale в том, что (главная идея):

       а) мы не создаём копий объекта для последующей нормализации при
          как минимум сравнении. И, наверное, получится так же сделать при сложении, вычитании и умножении.
       б) Из плюсов тут то, что не надо вектору-хранилищу лишний раз лазать в кучу и что-то аллоцировать - 
          операция не слишком лёгкая и сразу даст замедление работы
       в) Из минусов - код конкретного метода будет, наверное, чуть побольше, чем если тупо расширять BCD-числа.
          Но размер кода - обычно не проблема (Flash всегда жирнее ОЗУ), а вот ОЗУ обычно мало (на некторых 
          чипах - 20Кб и меньше), и больше, чем мало (на MCU).

          Куча (Heap) vs немного кода

          Код исполняется всегда, а в кучу лазаем не всегда ( у вектора есть capacity(), который в 
          итоге мы не сможем превысить до обращения в кучу)

          Код иполняется вегда, а в кучу лазаем редко. Что быбрать?
          Наверное, таки без копирования


     Намётки, как чё
     (уже неактуально, но пусть будет, покажет, если что, как блуждала моя мюсля)

     Сравнение
       сместить стартовые точки относительно заданной precision обоих векторов;
       если в одном из двух векторов idx выходит за пределы - то используем 0;
       диапазон индексов начинаем с наименьшего индекса с меньшей precision,
         большего - начинаем с наибольшего индекса макс разряда числа

       Сравнение возвращает -1, 0, 1

     Операторы (функции) сравнения
        Необходимы для реализации вычитания.
        Едионая  0 - провкрить н ноль обаслас гаемвх) UPD - чего я тут хотел сказать? :)

     Сложение
     Вычитание
       а) расширить точность до одинаковой (или сделать алгоритм сложнее без расширения)
       б) сместить стартовые точки относительно заданной precision обоих векторов;
          если в одном из двух векторов idx выходит за пределы - то используем 0;
          диапазон индексов начинаем с наименьшего индекса с меньшей precision,
            большего - начинаем с наибольшего индекса до макс разряда числа


          tckb (U1 <=> U2)
          пре сравнеии U1 - U2 сранения со .


     Умножение.
       Умножаем столбиком - не слишком эффективно, зато просто и я так уже делал.
       Про другие способы пишут, что они быстрее после ~250 знаков числа. Нам вроде такое не грозит в нормальной ситуации.

     Деление.
       Тут жопа - тут только делал тыщу лет назад что-то похожее.
       UPD Сделал тупо на вычитаниях и сдвигах - алгоритм практически тот же, что и при делении столбиком 
           (который все в начальной школе изучали)
           Просто при делении столбиком человек видит, что на что делится и надо ли сдвигать, и видит множитель, который
           и пишет в частное, а компьютер тупой, поэтому он в цикле вычитает.

     UPD Умножение и деление. 
         Нули в числах бывают, особенно в тех, которые задаёт человек (и тут очень и очень часто).
         Для нулевых цифр в числе все шаги алгоритма пропускаются


     ЗЫ Дёрнул же меня чёрт писать Decimal самому

     ЗЗЫ На несколько вечеров есть чем заняться :)


     //TODO: !!! Оптимизация умножения/деления на степень 10
     // В 'том случае ващет достаточно подвинуть десятичную точку. 
     // Поэтому надо проверять параметры, не являются ли они 10 в степени N.
     // Для умножения - оба, для деления - только делитель.
     // Проверка довольно копеечная, но если числа вида степень десяти очень редки,
     // то минус копеечка набежит, а профита не будет.
     // Надо будет сделать опционально.
 */


} // namespace bcd

} // namespace marty



