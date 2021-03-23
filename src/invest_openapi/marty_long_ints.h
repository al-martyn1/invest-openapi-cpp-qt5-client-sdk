#pragma once

#include <cstdint>
#include <climits>
#include <limits>

namespace marty
{



// cli/numeric.h
namespace int_helpers
{

template<typename IntType> inline
IntType makeCharSizeMask()
{
    return (IntType)(unsigned char)-1;
}

// https://en.cppreference.com/w/cpp/types/numeric_limits
// https://en.cppreference.com/w/cpp/types/climits
// template<> class numeric_limits<unsigned char>;


template<typename IntType> inline
IntType makeHighBitMask( )
{
    return ((IntType)1) << (numBits<IntType>()-1);
}


template<typename IntType> inline
unsigned numBits()
{
    return sizeof(IntType)*numBits<unsigned char>();
}

template<> inline
unsigned numBits<unsigned char>()
{
    return (unsigned)CHAR_BIT;
}

template<typename IntType> inline
std::size_t getHalfShiftSize()
{
    return (sizeof(IntType)*numBits<unsigned char>()) / 2;
}



template<typename IntType> inline
IntType makeMaskLowHalf()
{
    IntType mask = makeCharSizeMask<IntType>(); //  char_size_mask();
    IntType resMask = mask;
    for(unsigned i=0; i!=(unsigned)(sizeof(IntType)/2); ++i)
       {
        resMask |= mask;
        mask   <<= numBits<unsigned char>(); // char_bits();
       }
    return resMask;
}

//! Returns low part of integer in lower bits
template<typename IntType> inline
IntType getLowHalf( IntType i )
{
    return i & makeMaskLowHalf<IntType>();
}

//! Returns high part of integer in lower bits (shifted by size/2)
template<typename IntType> inline
IntType getHighPart( IntType i ) 
{
    return (i >> getHalfShiftSize<IntType>()) & makeMaskLowHalf<IntType>();
}

#if 0


template<typename IntType> inline
IntType getHighPartSigned( IntType i ) 
{
    if (i & makeHighBitMask<IntType>())
    {
        IntType res = getHighPart( i );
        res |= makeMaskLowHalf<IntType>() << getHalfShiftSize<IntType>();
        return res;
    }

    return getHighPart( i );;
}



// returns true if overflow detected
// IntType must be unsigned type
// TODO: !!! Need to be updated with new C++ features
template<IntType> inline
bool unsignedAddition( IntType a, IntType b, IntType *c = 0 )
{
    IntType r = a + b; if (c) *c = r; return !(r>=a && r>=b);
}

template<IntType> inline
bool unsignedAddition( IntType a, IntType b, IntType &c )
{
    IntType r = a + b; c = r; return !(r>=a && r>=b);
}

// returns true if carry detected
template<IntType> inline
bool unsignedSubtraction( IntType a, IntType b, IntType *c = 0 )
{
    IntType r = a - b; if (c) *c = r; return a<b;
}

template<IntType> inline
bool unsignedSubtraction( IntType a, IntType b, IntType &c )
{
    IntType r = a - b; c = r; return a<b;
}




template<typename UnsignedIntType1, typename UnsignedIntType2>
void unsignedMultiplyNumbersMnImpl( UnsignedIntType1 *w
           , const UnsignedIntType1 *u, const UnsignedIntType1 *v
           , unsigned m, unsigned n
           )
{
    UIntT2 k, t; //, b;
    unsigned i, j; // counters

    for (i=0; i<m; i++)
        w[i] = 0;

    for(j=0; j!=n; j++)
       {
        k = 0;
        for(i=0; i!=m; i++)
           {
            //t = u[i]   * v[j] 
            //  + w[i+j] + k;
            t = static_cast<UnsignedIntType1>(u[i])   * static_cast<UnsignedIntType1>(v[j]) 
              + static_cast<UnsignedIntType1>(w[i+j]) + static_cast<UnsignedIntType1>(k);
            w[i+j] = static_cast<UnsignedIntType1>(getLowHalf(t)); // signed? t; // (Т.е., t & OxFFFF).
            k = getHighPart(t); // signed? t >> 16;
           }
        w[j+m] = static_cast<UnsignedIntType1>(k);
       }
}


#endif




} // namespace int_helpers


//! Тупое умножение в столбик беззнаковых чисел произвольной длины, 1 разряд - 2 байта, младшие - сначала
/*! uint16_t используется как 1 разряд для того, чтобы два разряда влезало в 32 бита, что доступно на контроллерах
 */
inline
void miltipleUnsignedLongInts( const std::uint16_t *pM1 , std::size_t szM1
                             , const std::uint16_t *pM2 , std::size_t szM2
                             ,       std::uint16_t *pRes, std::size_t szResBuf
                             )
{
    const std::size_t resIdxEnd   = szResBuf;
    for( std::size_t k=0; k!=resIdxEnd; ++k)
       *pRes = 0;


    for( std::size_t i=0; i!=szM1; ++i)
    {
        if (!*pM1)
            continue;

        for( std::size_t j=0; j!=szM2; ++j)
        {
            std::size_t resIdxBegin = i+j;

            std::uint16_t m1 = *pM1;
            std::uint16_t m2 = *pM2;

            if (!m2)
               continue;

            std::uint32_t tmpRes = ((std::uint32_t)m1) * ((std::uint32_t)m2);

            for( std::size_t k=resIdxBegin; k!=resIdxEnd; ++k)
            {
                tmpRes += (std::uint32_t)pRes[k];
                pRes[k] = (std::uint16_t)tmpRes; // усекаем отрубанием старшей части

                tmpRes >>= (unsigned)int_helpers::getHalfShiftSize<std::uint32_t>();

                std::uint32_t tmp2 = 0;
                if ((k+1)<resIdxEnd)
                {
                    tmp2 = (std::uint32_t)pRes[k+1];
                }

                tmp2 <<= (unsigned)int_helpers::getHalfShiftSize<std::uint32_t>();

                tmpRes |= tmp2;
            }

            
        }
    }
}

// int_helpers::getHalfShiftSize()
// int_helpers::makeMaskLowHalf()



} // namespace marty


