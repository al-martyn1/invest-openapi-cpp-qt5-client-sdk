#pragma once


#include <cstdint>
#include <climits>
#include <limits>
#include <exception>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <algorithm>
#include <iterator>


namespace marty
{

namespace bcd
{


typedef signed char decimal_digit_t;


typedef std::vector<decimal_digit_t> raw_bcd_number_t; // without sign


//! Returns precision
inline
std::size_t makeRawBcdNumber( raw_bcd_number_t &bcdNumber, const char *str, std::size_t *pProcessedSymbols = 0)
{
    bcdNumber.clear();
    bcdNumber.reserve(std::strlen(str));

    std::size_t precision = 0;

    std::size_t processedSymbols = 0;

    bool processingFranctionalPart = false;

    for(; *str; ++str, ++processedSymbols /* , ++precision */  )
    {
        if ( (*str == '.') || (*str == ',') )
        {
            if (processingFranctionalPart)
            {
                // Already processing franctional part
                if (pProcessedSymbols)
                   *pProcessedSymbols = processedSymbols;
                std::reverse(bcdNumber.begin(), bcdNumber.end());
                return precision;
            }

            processingFranctionalPart = true;

            if (!processedSymbols)
            {
                bcdNumber.push_back(0);
            }

            continue;
        }

        if ( (*str < '0') || (*str > '9') )
        {
            if (pProcessedSymbols)
               *pProcessedSymbols = processedSymbols;

            std::reverse(bcdNumber.begin(), bcdNumber.end());
            return precision;
        }

        decimal_digit_t d = (decimal_digit_t)(*str - '0');

        bcdNumber.push_back(d);

        if (processingFranctionalPart)
            ++precision;
    }

    if (pProcessedSymbols)
       *pProcessedSymbols = processedSymbols;

    std::reverse(bcdNumber.begin(), bcdNumber.end());
    return precision;

}



inline
const char* formatRawBcdNumber( const raw_bcd_number_t &bcdNumber, std::size_t precision, char *pBuf, std::size_t bufSize, char sep = '.' )
{
    if (bufSize < (bcdNumber.size()+3)) // place for dot and final zero and leading zero digit
        throw std::runtime_error("marty::formatRawBcdNumber: bufSize is not enough");

    std::size_t i = 0, size = bcdNumber.size();

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
}


inline
std::size_t reducePrecision( raw_bcd_number_t &bcdNumber, std::size_t precision )
{
    std::size_t i = 0, size = bcdNumber.size();

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

inline
std::size_t reduceLeadingZeros( raw_bcd_number_t &bcdNumber, std::size_t precision )
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


} // namespace bcd

} // namespace marty



