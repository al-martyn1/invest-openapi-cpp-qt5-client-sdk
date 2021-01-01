#pragma once

#include <QObject>
#include <QVariant>
#include <QString>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlField>
#include <QSqlQuery>
#include <QSharedPointer>

#include <exception>
#include <stdexcept>


#include "database_manager_sqlite_impl.h"

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
namespace invest_openapi
{




//----------------------------------------------------------------------------
inline
QSharedPointer<IDatabaseManager> createDatabaseManager( QSharedPointer<QSqlDatabase> pDb, const DatabaseConfig &dbConfig )
{
    DatabaseConfig escapedDbConfig = dbConfig.escapeForDb( * pDb.get() );

    QString driverName = pDb->driverName();
    if (driverName=="")
    {
        throw std::runtime_error("invest_openapi::createDatabaseManager: Empty database driver name");
    }
    else if (driverName=="QSQLITE")
    {
        return QSharedPointer<IDatabaseManager>( new DatabaseManagerSQLiteImpl(pDb, dbConfig) );
    }

    throw std::runtime_error("invest_openapi::createDatabaseManager: Unknown database driver name");

    return QSharedPointer<IDatabaseManager>(0);
}

//----------------------------------------------------------------------------





} // namespace invest_openapi

