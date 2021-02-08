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
#include <set>
#include <algorithm>
#include <iterator>



#include "i_database_manager.h"
#include "logging_config.h"


//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
namespace invest_openapi
{



//----------------------------------------------------------------------------
class DatabaseManagerImplBase : public IDatabaseManager
{

protected:

    //------------------------------
    DatabaseManagerImplBase( QSharedPointer<QSqlDatabase> pDb, QSharedPointer<DatabaseConfig> pDatabaseConfig, QSharedPointer<LoggingConfig> pLoggingConfig )
    : m_pDatabaseConfig(pDatabaseConfig)
    , m_pLoggingConfig(pLoggingConfig)
    , m_pDb(pDb)
    {}

    //------------------------------
    DatabaseManagerImplBase();
    DatabaseManagerImplBase( const DatabaseManagerImplBase &);
    DatabaseManagerImplBase& operator=( const DatabaseManagerImplBase &);


    //------------------------------
    virtual QString tab() const override
    {
        return QString("                ");
    }

    virtual QString sqlQuote( const QString &str ) const override
    {
        QSqlField f(QLatin1String(""), QVariant::String);
        f.setValue(str);
        return m_pDb->driver()->formatValue(f);
    }

    virtual QString sqlQuote( const QVariant &str ) const override
    {
        if (str.isNull() || !str.isValid())
            return sqlQuote(QString());
        return sqlQuote(str.toString());
    }

    virtual QVector<QString> sqlQuote( const QVector<QString > &strs ) const override
    {
        QVector<QString> resVec;
        std::transform( strs.begin(), strs.end(), std::back_inserter(resVec)
                      , [this]( const QString &s )
                        {
                            return sqlQuote(s);
                        }
                      );
        return resVec;
    }

    virtual QVector<QString> sqlQuote( const QVector<QVariant> &strs ) const override
    {
        QVector<QString> resVec;
        std::transform( strs.begin(), strs.end(), std::back_inserter(resVec)
                      , [this]( const QVariant &s )
                        {
                            return sqlQuote(s);
                        }
                      );
        return resVec;
    }



    //------------------------------
    virtual QString getTableExistString( const QString &tableName ) const override
    {
        return QString("Table '") + tableName + QString("' ") + QString(tableCheckExist( tableName ) ? "EXISTS" : "NOT EXISTS" );
    }

    //------------------------------
    void checkTableName( const QString &tableName ) const
    {
        if (tableName.isEmpty())
            throw std::runtime_error("invest_openapi::DatabaseManagerSQLite: Invalid/unknown table name");
    }

    //------------------------------
    virtual std::size_t getQueryResultSize( QSqlQuery &query, bool needBool = false ) const override
    {
        if ( m_pDb->driver()->hasFeature(QSqlDriver::QuerySize))
        {
            return query.size();
        }
        else
        {
            // this can be very slow

            // Чёта это из КуДок/КуПримеров выцепленное не работает
            //query.last();
            //return query.at() + 1;

            // Поэтому вот такой вот самопал

            std::size_t cnt = 0;

            query.first();
            if (!query.isValid())
                return 0;

            do
            {
                cnt++;
                if (needBool)
                    return cnt;

            } while(query.next());

            return cnt;
        }
    }

    //------------------------------
    virtual void   setDefDecimal      ( unsigned total, unsigned frac ) override
    {
        m_defDecimalTotal = total;
        m_defDecimalFrac  = frac;
    }

    virtual void   applyDefDecimalFromConfig( const DatabaseConfig & cfg ) override
    {
        setDefDecimal( cfg.defaultDecimalFormatTotalSize
                     , cfg.defaultDecimalFormatFractionalSize
                     );
    }

    //------------------------------
    virtual QString tableMapName       ( const QString &tableName  ) const override // to internal name mapping
    {
        if (tableName.toUpper()=="INSTRUMENTS")
        {
            return m_pDatabaseConfig->tableNameInstruments;
        }

        //checkTableName(QString());
        //return QString();
        return QString('\'') + tableName + QString('\''); // return unmapped value
    }

    //------------------------------
    virtual QSqlQuery   execHelper ( const QString &queryText ) const override
    {
        QSqlQuery query(*m_pDb);
        if (!query.exec(queryText))
            return query;

        //query.first();
        return query;
    }

    //------------------------------
    virtual QVector<QString> queryToSingleStringVector( QSqlQuery& query, int valIdx, const QVector<QString> &except, bool caseCompare ) const override
    {
        QSet<QString> exceptsSet = makeSet( except, !caseCompare );

        QVector<QString> resVec;

        //if (!query.isValid())
        //    return resVec;

        //do
        while (query.next())
        {
            //int sz = query.record().count();// query.size();
            QString str = query.value(valIdx).toString();

            QString strCmp = str;
            if (!caseCompare)
                strCmp = str.toUpper();

            if (exceptsSet.find(strCmp)!=exceptsSet.end())
                continue;

            resVec.push_back(str);

        }    
        //} while(query.next());

        return resVec;
    }

    //------------------------------
    virtual QVector<QString> queryToSingleStringVector( QSqlQuery& query, int valIdx, const QStringList      &except, bool caseCompare ) const override
    {
        return queryToSingleStringVector( query, valIdx, toStringVector(except), caseCompare );
    }

    //------------------------------
    virtual QVector<QString> queryToSingleStringVector( QSqlQuery& query, int valIdx,       QString           except, bool caseCompare ) const override
    {
        return queryToSingleStringVector( query, valIdx, splitString( except, ".,:;" ), caseCompare );
    }

    //------------------------------
    virtual QVector<QString> queryToSingleStringVector( QSqlQuery& query, int valIdx ) const override
    {
        return queryToSingleStringVector( query, valIdx, QVector<QString>(), false );
    }


    /*
    //------------------------------
    virtual bool insertTo( const QString &tableName, const QVector<QVariant> &vals, const QVector<QString> &tableColumns ) const override
    {
        QVector<QVector<QVariant> > tmp; tmp.push_back(vals); return insertToImpl(tableName, tmp, tableColumns);
    }

    //------------------------------
    virtual bool insertTo( const QString &tableName, const QVector<QVariant> &vals, const QStringList      &tableColumns ) const override
    {
        QVector<QVector<QVariant> > tmp; tmp.push_back(vals); return insertToImpl(tableName, tmp, toStringVector(tableColumns) );
    }

    //------------------------------
    virtual bool insertTo( const QString &tableName, const QVector<QVariant> &vals,       QString           tableColumns ) const override
    {
        QVector<QVector<QVariant> > tmp; tmp.push_back(vals); return insertToImpl(tableName, tmp, toStringVector(splitString(tableColumns, ",.:")) );
    }

    //------------------------------
    virtual bool insertTo( const QString &tableName, const QVector<QString > &vals, const QVector<QString> &tableColumns ) const override
    {
        QVector<QVector<QString> > tmp; tmp.push_back(vals); return insertToImpl(tableName, tmp, tableColumns);
    }

    //------------------------------
    virtual bool insertTo( const QString &tableName, const QVector<QString > &vals, const QStringList      &tableColumns ) const override
    {
        QVector<QVector<QString> > tmp; tmp.push_back(vals); return insertToImpl(tableName, tmp, toStringVector(tableColumns) );
    }

    //------------------------------
    virtual bool insertTo( const QString &tableName, const QVector<QString > &vals,       QString           tableColumns ) const override
    {
        QVector<QVector<QString> > tmp; tmp.push_back(vals); return insertToImpl(tableName, tmp, toStringVector(splitString(tableColumns, ",.:")) );
    }

    //------------------------------


    virtual bool insertTo( const QString &tableName, const QVector<QVector<QVariant> > &vals, const QVector<QString> &tableColumns ) const override
    {
        return insertToImpl(tableName, vals, tableColumns);
    }

    virtual bool insertTo( const QString &tableName, const QVector<QVector<QVariant> > &vals, const QStringList      &tableColumns ) const override
    {
        return insertTo( tableName, vals, makeVector(tableColumns, false) );
    }

    virtual bool insertTo( const QString &tableName, const QVector<QVector<QVariant> > &vals,       QString           tableColumns ) const override
    {
        return insertTo( tableName, vals, splitString( tableColumns, ".,:;" ) );
    }

    virtual bool insertTo( const QString &tableName, const QVector<QVector<QString > > &vals, const QVector<QString> &tableColumns ) const override
    {
        return insertToImpl(tableName, vals, tableColumns);
    }

    virtual bool insertTo( const QString &tableName, const QVector<QVector<QString > > &vals, const QStringList      &tableColumns ) const override
    {
        return insertTo( tableName, vals, makeVector(tableColumns, false) );
    }

    virtual bool insertTo( const QString &tableName, const QVector<QVector<QString > > &vals,       QString           tableColumns ) const override
    {
        return insertTo( tableName, vals, splitString( tableColumns, ".,:;" ) );
    }
    */
    
    
    
    //------------------------------
    QSharedPointer<DatabaseConfig>    m_pDatabaseConfig;
    QSharedPointer<LoggingConfig>     m_pLoggingConfig;
    QSharedPointer<QSqlDatabase>      m_pDb;

    unsigned                          m_defDecimalTotal = 0;
    unsigned                          m_defDecimalFrac  = 0;

    //------------------------------

}; // class DatabaseManagerImplBase

//----------------------------------------------------------------------------




} // namespace invest_openapi

