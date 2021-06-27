#pragma once

//-----------------------------------------------------------------------------

/*! \file
    \brief Хелперы для enum'ов
*/

//-----------------------------------------------------------------------------


#include "stl.h"

//! Реализует битовые операции для enum-типа
#define UMBA_ENUM_CLASS_IMPLEMENT_FLAG_OPERATORS( EnumType )                                                    \
                                                                                                                \
inline                                                                                                          \
EnumType operator~( EnumType e )                                                                                \
{                                                                                                               \
    typedef typename std::underlying_type< EnumType >::type    EnumUnderlyingType;                              \
                                                                                                                \
    return static_cast<EnumType>( ~ static_cast<EnumUnderlyingType>(e) );                                       \
}                                                                                                               \
                                                                                                                \
inline                                                                                                          \
EnumType operator|( EnumType e1, EnumType e2 )                                                                  \
{                                                                                                               \
    typedef typename std::underlying_type< EnumType >::type    EnumUnderlyingType;                              \
                                                                                                                \
    return static_cast<EnumType>(static_cast<EnumUnderlyingType>(e1) | static_cast<EnumUnderlyingType>(e2));    \
}                                                                                                               \
                                                                                                                \
inline                                                                                                          \
EnumType operator&( EnumType e1, EnumType e2 )                                                                  \
{                                                                                                               \
    typedef typename std::underlying_type< EnumType >::type    EnumUnderlyingType;                              \
                                                                                                                \
    return static_cast<EnumType>(static_cast<EnumUnderlyingType>(e1) & static_cast<EnumUnderlyingType>(e2));    \
}                                                                                                               \
                                                                                                                \
inline                                                                                                          \
EnumType operator^( EnumType e1, EnumType e2 )                                                                  \
{                                                                                                               \
    typedef typename std::underlying_type< EnumType >::type    EnumUnderlyingType;                              \
                                                                                                                \
    return static_cast<EnumType>(static_cast<EnumUnderlyingType>(e1) ^ static_cast<EnumUnderlyingType>(e2));    \
}                                                                                                               \
                                                                                                                \
inline                                                                                                          \
EnumType operator<<( EnumType e, unsigned n )                                                                   \
{                                                                                                               \
    typedef typename std::underlying_type< EnumType >::type    EnumUnderlyingType;                              \
                                                                                                                \
    return static_cast<EnumType>(static_cast<EnumUnderlyingType>(e) << n );                                     \
}                                                                                                               \
                                                                                                                \
inline                                                                                                          \
EnumType operator>>( EnumType e, unsigned n )                                                                   \
{                                                                                                               \
    typedef typename std::underlying_type< EnumType >::type    EnumUnderlyingType;                              \
                                                                                                                \
    return static_cast<EnumType>(static_cast<EnumUnderlyingType>(e) << n );                                     \
}                                                                                                               \
                                                                                                                \
inline                                                                                                          \
EnumType& operator|=( EnumType &e1, EnumType e2 )                                                               \
{                                                                                                               \
    typedef typename std::underlying_type< EnumType >::type    EnumUnderlyingType;                              \
                                                                                                                \
    e1 = static_cast<EnumType>(static_cast<EnumUnderlyingType>(e1) | static_cast<EnumUnderlyingType>(e2));      \
    return e1;                                                                                                  \
}                                                                                                               \
                                                                                                                \
inline                                                                                                          \
EnumType& operator&=( EnumType &e1, EnumType e2 )                                                               \
{                                                                                                               \
    typedef typename std::underlying_type< EnumType >::type    EnumUnderlyingType;                              \
                                                                                                                \
    e1 = static_cast<EnumType>(static_cast<EnumUnderlyingType>(e1) & static_cast<EnumUnderlyingType>(e2));      \
    return e1;                                                                                                  \
}                                                                                                               \
                                                                                                                \
inline                                                                                                          \
EnumType& operator^=( EnumType &e1, EnumType e2 )                                                               \
{                                                                                                               \
    typedef typename std::underlying_type< EnumType >::type    EnumUnderlyingType;                              \
                                                                                                                \
    e1 = static_cast<EnumType>(static_cast<EnumUnderlyingType>(e1) ^ static_cast<EnumUnderlyingType>(e2));      \
    return e1;                                                                                                  \
}                                                                                                               \
                                                                                                                \
inline                                                                                                          \
EnumType& operator<<=( EnumType &e, unsigned n )                                                                \
{                                                                                                               \
    typedef typename std::underlying_type< EnumType >::type    EnumUnderlyingType;                              \
                                                                                                                \
    e = static_cast<EnumType>(static_cast<EnumUnderlyingType>(e) << n );                                        \
    return e;                                                                                                   \
}                                                                                                               \
                                                                                                                \
inline                                                                                                          \
EnumType& operator>>=( EnumType &e, unsigned n )                                                                \
{                                                                                                               \
    typedef typename std::underlying_type< EnumType >::type    EnumUnderlyingType;                              \
                                                                                                                \
    e = static_cast<EnumType>(static_cast<EnumUnderlyingType>(e) << n );                                        \
    return e;                                                                                                   \
}                                                                                                               \
                                                                                                                \
inline                                                                                                          \
bool operator!(EnumType e)                                                                                      \
{                                                                                                               \
    typedef typename std::underlying_type< EnumType >::type    EnumUnderlyingType;                              \
                                                                                                                \
    return static_cast<EnumUnderlyingType>(e)==0;                                                               \
}






namespace umba
{


#if !defined(UMBA_MCU_USED)

//! Генерирует вектор последовательных значений enum'а
template< typename EnumType >
inline
std::vector<EnumType> enumValuesToVector( EnumType eb, EnumType ee, bool inclusiveEnd = false )
{
    // size_t b = (size_t)eb;
    // size_t e = (size_t)ee;

    typedef typename std::underlying_type< EnumType >::type    EnumUnderlyingType;
    EnumUnderlyingType b = (EnumUnderlyingType)eb;
    EnumUnderlyingType e = (EnumUnderlyingType)ee;
    if (inclusiveEnd)
        ++e;

    std::vector<EnumType> res;
    res.reserve( e-b );

    for( ; b!=e; ++b)
        res.push_back( (EnumType)b );

    return res;

}

#endif // UMBA_MCU_USED


} // namespace umba

