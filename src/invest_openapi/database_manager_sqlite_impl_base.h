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
#include <algorithm>
#include <iterator>

#include "database_manager_impl_base.h"
#include "model_to_strings.h"

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
    virtual QVector<QString> tableGetNamesFromDb() const override
    {
        QSqlQuery query = execHelper("SELECT NAME FROM sqlite_master where type=\'table\';");
        return queryToSingleStringVector( query, 0, "sqlite_sequence", false );
    }
    
    //------------------------------
    virtual QVector<QString> tableGetColumnsFromDbInternal( const QString &internalTableName ) const override
    {
        QSqlQuery query = execHelper(QString("PRAGMA table_info(%1);").arg(sqlQuote(internalTableName)));
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

        QVector<QString> additionalUniques = tableGetAdditionalUniques( tableName );

        QString queryText;

        if (additionalUniques.empty())
        {
            queryText = QString("CREATE TABLE %1 %2 (%3%4%5)").arg(exists(existence))
                                                              .arg(tableMapName(tableName))
                                                              .arg(lf())
                                                              .arg(tableGetSchema(tableName))
                                                              .arg(lf())
                                                              ;
        }
        else
        {
            queryText = QString("CREATE TABLE %1 %2 (%3%4,%5%6%7)").arg(exists(existence))
                                                                   .arg(tableMapName(tableName))
                                                                   .arg(lf())
                                                                   .arg(tableGetSchema(tableName))
                                                                   .arg(lf())
                                                                   .arg(mergeString(additionalUniques, ", "))
                                                                   .arg(lf())
                                                                   ;
        }

        
        RETURN_IOA_SQL_EXEC_QUERY( query, queryText );
        // return query.exec( queryText );
    }

    virtual QString     makeSimpleSelectQueryText( const QString &tableName
                                                 , const QVector<QString > &fields = QVector<QString >() ) const override
    {
        if (!fields.empty())
            return QString("SELECT %1 FROM %2").arg( mergeString(fields, ", ") ).arg(tableName);
        else
            return QString("SELECT %1 FROM %2").arg( mergeString(tableGetColumnsFromSchema(tableName), ", ") ).arg(tableName);
    }

    virtual QString     makeSimpleSelectQueryText( const QString &tableName
                                                 , const QVector<QString> &whereNames, const QVector<QString> &whereVals
                                                 , const QVector<QString > &fields = QVector<QString >() ) const override
    {
        if ( whereNames.size() != whereVals.size() )
             throw std::runtime_error("makeSimpleSelectQueryText: whereNames.size() != whereVal.size()");

        QString queryText = makeSimpleSelectQueryText( tableName, fields );

        if (whereNames.size()==0)
            return queryText;

        QVector<QString> whereVec;

        for( int idx=0; idx<whereNames.size(); ++idx)
        {
            whereVec.push_back( QString("%1 = %2").arg(whereNames[idx]).arg(sqlQuote(whereVals[idx])) );
        }

        return QString("%1 WHERE %2").arg(queryText)
                                     .arg(mergeString(whereVec, " AND "));
    }

    virtual QString     makeSimpleUpdateQueryText( const QString &tableName, const QString &whereName, const QString &whereVal, const QVector<QString > &values, QVector<QString > fields = QVector<QString >() ) const override
    {
        if (fields.empty())
            fields = tableGetColumnsFromSchema(tableName);

        QVector< QString > setPairs;

        int idx = 0;

        for(; idx<fields.size() && idx<values.size(); ++idx )
        {
            setPairs.push_back( QString("%1 = %2").arg(fields[idx]).arg(sqlQuote(values[idx])) );
        }

        QString queryText = QString("UPDATE %1 SET %2").arg(tableName).arg(mergeString(setPairs, ", "));
        if (whereName.isEmpty() || whereVal.isEmpty())
            return queryText;

        return queryText + QString(" WHERE %1 = %2").arg(whereName).arg(sqlQuote(whereVal));
    }

    virtual QString     makeSelectSingleDateQuery( const QString &queryText, const QString &dateField, bool fLast /* true for last, false for first */ ) const override
    {
        return QString("%1%2ORDER BY %3 %4%5LIMIT 1 OFFSET 0")
                      .arg(queryText)
                      .arg(lf())
                      .arg(dateField)
                      .arg( fLast ? "DESC" : "ASC" )
                      .arg(lf());
    }


    virtual bool insertToImpl( const QString &tableName, const QVector<QVector<QString> >  &vals, const QVector<QString> &tableColumns ) const override
    {
        QVector<QString> valuesList;

        int columnsSize = tableColumns.size();

        std::transform( vals.begin(), vals.end(), std::back_inserter(valuesList)
                      , [this, columnsSize, tableName]( const QVector<QString> &v )
                        {
                            if (columnsSize!=v.size())
                            {

                                throw std::runtime_error( std::string("DatabaseManagerSQLiteImplBase::insertToImpl: "
                                                                      "number of columns mismatch number of taken values for table ")
                                                        + tableName.toStdString()
                                                        );
                            }

                            return QString("(%1)").arg(mergeString( sqlQuote(v), ", "));
                        }
                      );

        QString queryText = QString("INSERT INTO %1 (%2)%3VALUES %5").arg(tableMapName(tableName))
                                                                     .arg( mergeString(tableColumns, ", ") )
                                                                     .arg(lf())
                                                                     .arg(mergeString( valuesList, QString(",") + lf() + QString("       ")) )
                                                                     ;
        QSqlQuery query(*m_pDb);
        RETURN_IOA_SQL_EXEC_QUERY( query, queryText );
    }

    /*
    virtual bool insertToImpl( const QString &tableName, const QVector<QString>   &vals, const QVector<QString> &tableColumns ) const override
    {
        QVector<QVector<QString> > tmp; tmp.push_back(vals);
        return insertToImpl( tableName, tmp, tableColumns );
    }

    virtual bool insertToImpl( const QString &tableName, const QVector<QVector<QVariant> > &vals, const QVector<QString> &tableColumns ) const override
    {
        return insertToImpl( tableName, toStringVector(vals), tableColumns );
    }

    virtual bool insertToImpl( const QString &tableName, const QVector<QVariant> &vals, const QVector<QString> &tableColumns ) const override
    {
        return insertToImpl( tableName, toStringVector(vals), tableColumns );
    }
    */

    //------------------------------
    virtual bool      metaInsertForTablesBulk ( const QString &bulkText ) const override
    {
        //QStringList = toStringVector(splitString(bulkText, ";"));
        auto vec = splitString(bulkText, ";", ",.:;");
        return insertTo( "_META_TABLES", splitString(bulkText, ";", ",.:;"), tableGetColumnsFromDb("_META_TABLES") /* "TABLE_NAME;DISPLAY_NAME;DESCRIPTION" */  );
    }

    //------------------------------
    virtual bool      metaInsertForColumnBulk ( QString tableName, const QString &bulkText ) const override
    {
        auto vec = splitString(bulkText, ";", ",.:;");
        return insertTo( "_META_COLUMNS", splitString(bulkText, ";", ",.:;"), tableGetColumnsFromDb("_META_COLUMNS") /* "TABLE_NAME;DISPLAY_NAME;DESCRIPTION" */  );
    }



}; // DatabaseManagerSQLiteImplBase

//----------------------------------------------------------------------------




} // namespace invest_openapi

