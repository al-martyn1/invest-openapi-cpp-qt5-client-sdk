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


#include "database_config.h"


namespace invest_openapi
{


QSharedPointer<IDatabaseManager> createDatabaseManager( QSharedPointer<QSqlDatabase> pDb, const DatabaseConfig &dbConfig );





struct IDatabaseManager
{

    virtual QString tableMapName       ( const QString &tableName  ) const = 0; // to internal name mapping
    virtual QString tableGetShema      ( const QString &tableName  ) const = 0;
    virtual bool    tableCheckExist    ( const QString &tableName  ) const = 0;
    virtual bool    tableDrop          ( const QString &tableName  ) const = 0;
    virtual bool    tableCreate        ( const QString &tableName  ) const = 0;

};






class DatabaseManagerBase : public IDatabaseManager
{

protected:

    DatabaseManagerBase( QSharedPointer<QSqlDatabase> pDb, const DatabaseConfig &databaseConfig )
    : m_databaseConfig(databaseConfig)
    , m_pDb(pDb)
    {}

    DatabaseManagerBase();
    DatabaseManagerBase( const DatabaseManagerBase &);
    DatabaseManagerBase& operator=( const DatabaseManagerBase &);

    DatabaseConfig                    m_databaseConfig;
    QSharedPointer<QSqlDatabase>      m_pDb;


    static
    std::size_t getQueryResultSize( QSqlQuery &q, bool needBool = false )
    {
        std::size_t cnt = 0;

        // Assumed that query is new or after q.first()

        while (q.isValid())
        {
            cnt++;
            if (needBool && cnt)
                break;
            q.next();
        }

        return cnt;

        /*
        while (query.next())
        {
            cnt++;
        }
        */

    }



};






class DatabaseManagerSQLite : public DatabaseManagerBase
{

protected:

    friend QSharedPointer<IDatabaseManager> createDatabaseManager( QSharedPointer<QSqlDatabase> pDb, const DatabaseConfig &dbConfig );

    void checkTableName( const QString &tableName ) const
    {
        if (tableName.isEmpty())
            throw std::runtime_error("invest_openapi::DatabaseManagerSQLite: Invalid/unknown table name");
    }

    DatabaseManagerSQLite( QSharedPointer<QSqlDatabase> pDb, const DatabaseConfig &databaseConfig )
    : DatabaseManagerBase(pDb, databaseConfig)
    {}

    virtual QString tableMapName       ( const QString &tableName  ) const override // to internal name mapping
    {
        if (tableName.toUpper()=="INSTRUMENTS")
        {
            return m_databaseConfig.tableNameInstruments;
        }

        checkTableName(QString());

        return QString();
    }


    virtual QString tableGetShema      ( const QString &tableName  ) const override
    {
        return QString();
    }

    virtual bool    tableCheckExist    ( const QString &tableName  ) const override
    {
        // https://stackoverflow.com/questions/1601151/how-do-i-check-in-sqlite-whether-a-table-exists
        // sqlite_temp_master.
        // SELECT count(*) FROM sqlite_master WHERE type='table' AND name='table_name';
        // SELECT 1

        QSqlQuery query(*m_pDb);
        query.exec(QString("SELECT name FROM sqlite_master WHERE type='table' AND name='") + tableMapName(tableName) + QString("';"));
        //query.exec(QString("SELECT 1 FROM sqlite_master WHERE type='table' AND name='") + tableMapName(tableName) + QString("';"));

        if (getQueryResultSize( query, true /* needBool */ ))
            return true;

        return false;
    }

    virtual bool    tableDrop          ( const QString &tableName  ) const override
    {
        QSqlQuery query(*m_pDb);
        query.exec(QString("DROP TABLE ") + tableMapName(tableName) + QString(";") );
        return true;
    }

    virtual bool    tableCreate        ( const QString &tableName  ) const override
    {
        QSqlQuery query(*m_pDb);
        //create table if not exists
        query.exec( QString("CREATE TABLE ") 
                  + tableMapName(tableName)
                  + tableGetShema(tableName)
                  );
        return true;
    }

};


/*

    bool createDatabaseShema(QSqlDatabase &db) const
    {
        QSqlQuery query(db);

        query.exec(QString("DROP TABLE ") + tableNameInstruments + QString(";") );


        query.exec( QString("CREATE TABLE ") + tableNameInstruments + QString(""
            "(id integer primary key, "
            "firstname varchar(20), "
            "lastname varchar(30), "
            "age integer)"
            )
        );

        return true;
        
    }

    QString getTableShema( const QString &tableInternalName ) const
    {}



    // SELECT name FROM sqlite_master WHERE type='table' AND name='{table_name}';

*/


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
        return QSharedPointer<IDatabaseManager>( new DatabaseManagerSQLite(pDb, dbConfig) );
    }

    throw std::runtime_error("invest_openapi::createDatabaseManager: Unknown database driver name");

    return QSharedPointer<IDatabaseManager>(0);
}


} // namespace invest_openapi

