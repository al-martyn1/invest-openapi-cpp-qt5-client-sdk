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


template<typename T> inline
T makeHighBitMask( T )
{

}


template<typename T> inline
unsigned numBits()
{
    return sizeof(T)*numBits<unsigned char>();
}

template<> inline
unsigned numBits<unsigned char>()
{
    return (unsigned)CHAR_BIT;
}

template<typename IntT> inline
std::size_t getHalfShiftSize()
{
    return (sizeof(IntT)*numBits<unsigned char>()) / 2;
}



template<typename IntT> inline
IntT makeMaskLowHalf()
{
    IntT mask = makeCharSizeMask<IntT>(); //  char_size_mask();
    IntT resMask = mask;
    for(unsigned i=0; i!=(unsigned)(sizeof(IntT)/2); ++i)
       {
        resMask |= mask;
        mask   <<= numBits<unsigned char>(); // char_bits();
       }
    return resMask;
}

//! Returns low part of integer in lower bits
template<typename IntT> inline
IntT getLowHalf( IntT i )
{
    return i & makeMaskLowHalf<IntT>();
}

//! Returns high part of integer in lower bits (shifted by size/2)
template<typename IntT> inline
IntT getHighPart( IntT i ) 
{
    return (i >> getHalfShiftSize<IntT>()) & makeMaskLowHalf<IntT>();
}

template<typename IntT> inline
IntT getHighPartSigned( IntT i ) 
{
    return 
    
    IntT res = ( i >> getHalfShiftSize<IntT>()) & makeMaskLowHalf<IntT>();



         | ( makeMaskLowHalf<IntT>() << getHalfShiftSize<IntT>() )
           ;
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








} // namespace int_helpers






} // namespace marty


