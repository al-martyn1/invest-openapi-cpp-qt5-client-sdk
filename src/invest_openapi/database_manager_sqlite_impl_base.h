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


#include "database_manager_impl_base.h"

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
namespace invest_openapi
{


//----------------------------------------------------------------------------
class DatabaseManagerSQLiteImplBase : public DatabaseManagerImplBase
{

protected:

    //------------------------------
    friend QSharedPointer<IDatabaseManager> createDatabaseManager( QSharedPointer<QSqlDatabase> pDb, QSharedPointer<DatabaseConfig> pDatabaseConfig, QSharedPointer<LoggingConfig> pLoggingConfig );


    //------------------------------
    DatabaseManagerSQLiteImplBase( QSharedPointer<QSqlDatabase> pDb, QSharedPointer<DatabaseConfig> pDatabaseConfig, QSharedPointer<LoggingConfig> pLoggingConfig )
    : DatabaseManagerImplBase(pDb, pDatabaseConfig, pLoggingConfig)
    {}

    //------------------------------
    virtual QString   defDecimal( ) const override
    {
        return defDecimal( m_defDecimalTotal, m_defDecimalFrac );
    }
    //------------------------------

    virtual QString q( const QString &str ) const override
    {
        QString escapedStr = sqlEscape( *m_pDb, str );

        if (escapedStr.isEmpty())
            return QString("''");

        if (escapedStr.front()=='\'')
            return escapedStr;

        return QString("'")
             + escapedStr
             + QString("'")
             ;
    }

    //------------------------------
    virtual QString lf( char comma = ' ' ) const override
    {
        return QString("\r\n")
             + QString( (QChar)comma )
             + QString(" ")
             ;
    }

    //------------------------------
    virtual QString   defDecimal      ( unsigned total, unsigned frac ) const override
    {
        return QString("DECIMAL(%1,%2)").arg(total).arg(frac);
    }

    //------------------------------
    virtual bool    tableCheckExist    ( const QString &tableName  ) const override
    {
        // https://stackoverflow.com/questions/1601151/how-do-i-check-in-sqlite-whether-a-table-exists

        QString queryText = QString("SELECT name FROM sqlite_master WHERE type='table' AND name=") + tableMapName(tableName) + QString(";");
        QSqlQuery query(*m_pDb);

        IOA_SQL_EXEC_QUERY( query, queryText );

        if (getQueryResultSize( query, true /* needBool */ ))
            return true;

        return false;
    }

    //------------------------------
    QString exists( IDatabaseManager::IfExists e ) const
    {
        switch(e)
        {
            case IDatabaseManager::IfExists::ifExists   :     return QString("IF EXISTS");
            case IDatabaseManager::IfExists::ifNotExists:     return QString("IF NOT EXISTS");
            default                                     :     return QString("");
        }
    }

    //------------------------------
    virtual QVector<QString> tableGetNames    () const override
    {
        QSqlQuery query = selectExecHelper("SELECT NAME FROM sqlite_master where type=\'table\';");
        return queryToSingleStringVector( query, 0, "sqlite_sequence", false );
    }
    
    //------------------------------
    virtual QVector<QString> tableGetColumnsInternal ( const QString &internalTableName ) const override
    {
        QSqlQuery query = selectExecHelper(QString("PRAGMA table_info(%1);").arg(escape(internalTableName)));
        return queryToSingleStringVector( query, 1 );
    }

    //------------------------------
    virtual bool    tableDrop          ( const QString &tableName, IfExists existence ) const override
    {
        // https://www.sqlitetutorial.net/sqlite-drop-table/
        QSqlQuery query(*m_pDb);
        QString queryText = QString("DROP TABLE %1 %2;").arg(exists(existence)).arg(tableMapName(tableName));
        RETURN_IOA_SQL_EXEC_QUERY( query, queryText );
        // return query.exec( queryText );
    }

    //------------------------------
    virtual bool    tableCreate        ( const QString &tableName, IfExists existence ) const override
    {
        QSqlQuery query(*m_pDb);
        QString queryText = QString("CREATE TABLE %1 %2 (%3)").arg(exists(existence)).arg(tableMapName(tableName)).arg(tableGetShema(tableName));
        RETURN_IOA_SQL_EXEC_QUERY( query, queryText );
        // return query.exec( queryText );
    }

    //------------------------------
    virtual bool      metaInsertForTableExact  ( QString tableName, QString tableDisplayName, QString tableDescription ) const override
    {
        return true;
    }

    //------------------------------
    virtual bool      metaInsertForColumnExact ( QString tableName, QString columnName, QString tableDisplayName, QString tableDescription ) const override
    {
        return true;
    }

    //------------------------------
    virtual bool      metaInsertForTable       ( QStringTriple tripple ) const override
    {
        return true;
    }

    //------------------------------
    virtual bool      metaInsertForColumn      ( QStringQuatro quatro  ) const override
    {
        return true;
    }

    //------------------------------
    virtual bool      metaInsertForTableBulk   ( const QString &bulkText ) const override
    {
        return true;
    }

    //------------------------------
    virtual bool      metaInsertForColumnBulk  ( const QString &bulkText ) const override
    {
        return true;
    }



}; // DatabaseManagerSQLiteImplBase

//----------------------------------------------------------------------------




} // namespace invest_openapi

