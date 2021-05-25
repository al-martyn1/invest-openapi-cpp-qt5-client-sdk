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



#define INVEST_OPENAPI_OPEN_DATABASE_EX2( sqlDbVarName, dbManagerVarName, dbManagerCreationProc, logConfigVarName, dbConvigVarName, dbFilenameVarName, retValOnFail, driverName ) \
                QSharedPointer<QSqlDatabase> sqlDbVarName = QSharedPointer<QSqlDatabase>( new QSqlDatabase(QSqlDatabase::addDatabase(driverName, invest_openapi::generateNewDbConnectionName())) ); \
                sqlDbVarName->setDatabaseName( dbConvigVarName -> dbFilenameVarName );                                                               \
                if (!sqlDbVarName->open())                                                                                                           \
                {                                                                                                                                    \
                  qDebug().nospace().noquote()                                                                                                       \
                           << "Failed to open Database (" #dbFilenameVarName "), "                                                                   \
                           << "DB file: "                                                                                                            \
                           << dbConvigVarName -> dbFilenameVarName                                                                                   \
                           << ", Error: " << sqlDbVarName->lastError().text();                                                                       \
                  return retValOnFail;                                                                                                               \
                }                                                                                                                                    \
                                                                                                                                                     \
                QSharedPointer<invest_openapi::IDatabaseManager> dbManagerVarName                                                                    \
                              = invest_openapi:: dbManagerCreationProc( sqlDbVarName, dbConvigVarName, logConfigVarName );                           \
                                                                                                                                                     \
                dbManagerVarName -> applyDefDecimalFormatFromConfig( * dbConvigVarName )



#define INVEST_OPENAPI_OPEN_DATABASE_EX( sqlDbVarName, dbManagerVarName, dbManagerCreationProc, logConfigVarName, dbConvigVarName, dbFilenameVarName, retValOnFail ) \
                INVEST_OPENAPI_OPEN_DATABASE_EX2( sqlDbVarName, dbManagerVarName, dbManagerCreationProc, logConfigVarName, dbConvigVarName, dbFilenameVarName, retValOnFail, "QSQLITE" )


#define INVEST_OPENAPI_OPEN_DATABASE( sqlDbVarName, dbManagerVarName, dbManagerCreationProc, logConfigVarName, dbConvigVarName, dbFilenameVarName ) \
                INVEST_OPENAPI_OPEN_DATABASE_EX( sqlDbVarName, dbManagerVarName, dbManagerCreationProc, logConfigVarName, dbConvigVarName, dbFilenameVarName, 1 )







//----------------------------------------------------------------------------
namespace invest_openapi
{



//----------------------------------------------------------------------------
inline 
QString generateNewDbConnectionName()
{
    // qt_sql_default_connection
    static unsigned connectionCounter = 0;

    QString connectionName = QString("invest_openapi_qt_sql_default_connection_") + QString::number( connectionCounter );

    ++connectionCounter;

    return connectionName;
}

//----------------------------------------------------------------------------




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

