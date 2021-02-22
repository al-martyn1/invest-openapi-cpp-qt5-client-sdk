/*! \file
    \brief 
 */

#pragma once

#include <QObject>
#include <QDebug>
#include <QtDebug>
#include <QTest>
#include <QTimeZone>
#include <QDate>
#include <QTime>
#include <QDateTime>

#include <exception>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <map>

#include "invest_openapi.h"
#include "i_database_manager.h"


//----------------------------------------------------------------------------
namespace invest_openapi
{



//----------------------------------------------------------------------------
template< typename K, typename V >
inline
bool dictionaryGetValue( const std::map<K,V> &d, V &vOut, const K &k )
{
    std::map<K,V>::const_iterator it = d.find(k);
    if (it==d.end())
        return false;

    vOut = it->second;

    return true;
}

//----------------------------------------------------------------------------
template< typename K, typename V >
inline
bool dictionaryGetValue( const std::map<K,V> &d, V &vOut, const K &k, const V &vDefault )
{
    std::map<K,V>::const_iterator it = d.find(k);
    if (it==d.end())
        vOut = vDefault;
    else
        vOut = it->second;

    return true;
}

//----------------------------------------------------------------------------
template< typename K, typename V >
inline
V dictionaryGetValue( const std::map<K,V> &d, const K &k )
{
    V v;

    if (!dictionaryGetValue( d, v, k))
        throw std::runtime_error("Something goes wrong in dictionaryGetValue");
    
    return v;
}
//----------------------------------------------------------------------------






//----------------------------------------------------------------------------

} // namespace invest_openapi

