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

//----------------------------------------------------------------------------


/* Все функции/операции в marty::bcd неготовы к неконсистентным данным
   и предназначены только для внутреннего использования

 */


//----------------------------------------------------------------------------
namespace marty
{




//----------------------------------------------------------------------------
namespace bcd
{




//----------------------------------------------------------------------------

typedef signed char     decimal_digit_t;
typedef unsigned char   decimal_udigit_t;

typedef std::vector<decimal_digit_t> raw_bcd_number_t; // without sign

//----------------------------------------------------------------------------




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

    while(strSize!=0 && str[strSize-1]=='0') // "remove" trailing zeros
        --strSize;

    while( strSize!=0 && *str=='0') // "remove" leading zeros
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
//! Обрезаем точность, если возможно (в хвосте одни нули)
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
    while( !bcdNumber.empty() && (bcdNumber.back()==0) && (bcdNumber.size() > (precision+1)) )
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

        if (d1<d2)
            return -1;

        if (d1>d2)
            return  1;

    }

    return 0;
}

//----------------------------------------------------------------------------
//! Форматирование "сырого" BCD, форматируется в строку с заданной точностью
inline
const char* formatRawBcdNumber( const raw_bcd_number_t &bcdNumber, int precision, char *pBuf, std::size_t bufSize, char sep = '.' )
{
    int ipSize1      = getIntegerPartSize( bcdNumber, precision );
                                                                    
    int maxIpSize    = ipSize1;   

    int precision1   = precision;
                                                                    
    int maxPrecision = getMaxPrecision( precision1, 0 );
    int minPrecision = getMinPrecision( precision1, 0 );
    int dtPrecision  = maxPrecision - minPrecision;                 
                                                                    
    int offset1      = maxPrecision - precision1;                   

    int totalSize    = maxIpSize; /* virtual size */
    if (maxPrecision>0)
        totalSize += maxPrecision;


    if ((int)bufSize < (int)(totalSize+3) )
        throw std::runtime_error("marty::formatRawBcdNumber: bufSize is not enough");

    char *pBufBegin = pBuf;

    //bool digitsPrintedAfterDot = false;

    for( int idxFromBegin=0; idxFromBegin!=totalSize; ++idxFromBegin)
    {
        int idx  = (int)idxFromBegin;

        int idx1 = totalSize - idx - 1;

        idx1 -= (int)maxPrecision - (int)precision1;

        decimal_digit_t d1 = 0;

        if (idx1>=0 && idx1<(int)bcdNumber.size())
            d1 = bcdNumber[idx1];

        if (precision>0)
        {
            //int dotIndex = idx1 - precision1 - 1;
            //if (dotIndex==0) // dot pos found
            {
                //*pBuf++ = sep;
                //digitsPrintedAfterDot = false;
            }
        }

        *pBuf++ = d1 + '0';

        if (precision>0 && idxFromBegin==(maxIpSize-1))
        {
            *pBuf++ = sep;
        }

        
    }

    *pBuf = 0;

    //std::reverse(pBufBegin, pBuf);

    return pBufBegin;


    /*
    int i = 0, size = (int)bcdNumber.size();

    char *pBufBegin = pBuf;

    for(; i!=size; ++i, ++pBuf)
    {
        if (i!=0 && i==precision)
        {
            *pBuf++ = sep;
        }

        *pBuf = bcdNumber[i] + '0';
    }

    if (size==precision)
    {
        *pBuf++ = sep;
        *pBuf++ = '0'; // add leading zero digit
    }

    *pBuf = 0;

    std::reverse(pBufBegin, pBuf);

    return pBufBegin;
    */
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



     Сравнение
       сместить стартовые точки относительно заданной precision обоих векторов;
       если в одном из двух векторов idx выходит за пределы - то используем 0;
       диапазон индексов начинаем с наименьшего индекса с меньшей precision,
         большего - начинаем с наибольшего индекса макс разряда числа

       Сравнение возвращает -1, 0, 1

     Операторы (функции) сравнения
        Необходимы для реализации вычитания.
        Едионая  0 - провкрить н ноль обаслас гаемвх)

     Сложение
     Вычитание
       а) расширить точность до одинаковой (или сделать алгоритм сложнее без раширения)
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

     


     ЗЫ Дёрнул же меня чёрт писать Decimal самому

     ЗUU наибольшегонесколько вереров есть чем занчься
 */


} // namespace bcd

} // namespace marty



