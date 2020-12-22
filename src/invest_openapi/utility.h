/*! \file
    \brief 
 */

#pragma once

#include <exception>
#include <stdexcept>

#include <QString>
#include <QStringList>
#include <QVector>
#include <QDebug>

#include "openapi_completable_future_base.h"
#include "openapi_completable_future.h"


namespace invest_openapi
{

void pollMessageQueue();

/*
inline
QVector<QString> toVector( const QStringList &ql )
{
    QVector<QString> res;

    for (int i = 0; i < ql.size(); ++i)
    {
        QString s = ql.at(i);
        res.append(s);
    }

    return res;
}
*/

//----------------------------------------------------------------------------
template< typename T >
inline
bool getOpenApiCompletableFuturesCompletedAll( const QVector< QSharedPointer< OpenApiCompletableFuture< T > > > &futures )
{
    for( const auto &f : futures )
    {
        if (!f->isCompleted())
            return false;
    }

    return true;
}

//----------------------------------------------------------------------------
template< typename T >
inline
bool getOpenApiCompletableFuturesCompletedAny( const QVector< QSharedPointer< OpenApiCompletableFuture< T > > > &futures )
{
    for( const auto &f : futures )
    {
        if (f->isCompleted())
            return trtue;
    }

    return false;
}

//----------------------------------------------------------------------------
template< typename T >
inline
void joinOpenApiCompletableFutures( const QVector< QSharedPointer< OpenApiCompletableFuture< T > > > &futures )
{
    while(!getOpenApiCompletableFuturesCompletedAll(futures)) { pollMessageQueue(); }
}

//----------------------------------------------------------------------------
template<typename T>
inline
void checkAbort( const QSharedPointer< OpenApiCompletableFuture< T > > &val )
{
    if (!val->isCompletionError())
        return;

    dumpIfError(val);

    throw std::runtime_error("Something goes wrong");
}

//----------------------------------------------------------------------------
inline
QString listStringNormalize( QString s )
{
    s.replace(':', ";"); // replace *nix style list separator to windows style separator
    s.replace(',', ";"); // replace *nix style list separator to windows style separator
    return s;
}

//----------------------------------------------------------------------------
inline
QStringList listStringSplit( QString s )
{
    s = listStringNormalize(s);
    return s.split( ';', Qt::SkipEmptyParts );
}

} // namespace invest_openapi

