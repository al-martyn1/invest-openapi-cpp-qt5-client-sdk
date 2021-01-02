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

    virtual QString escape( const QString &str ) const override
    {
        QSqlField f(QLatin1String(""), QVariant::String);
        f.setValue(str);
        return m_pDb->driver()->formatValue(f);
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
    virtual QSqlQuery   selectExecHelper ( const QString &queryText ) const override
    {
        QSqlQuery query(*m_pDb);
        if (!query.exec(queryText))
            return query;

        query.first();
        return query;
    }

    //------------------------------
    virtual QVector<QString> queryToSingleStringVector( QSqlQuery& query, int valIdx, const QVector<QString> &except, bool caseCompare ) const override
    {
        std::set<QString> exceptsSet;

        for( QVector<QString>::const_iterator eit = except.begin(); eit != except.end(); ++eit )
        {
            #if defined(_DEBUG) || defined(DEBUG)
            QString curStr = caseCompare ? *eit : eit->toUpper();
            #endif

            exceptsSet.insert( caseCompare ? *eit : eit->toUpper() );
        }

        QVector<QString> resVec;

        if (!query.isValid())
            return resVec;

        do
        {
            //int sz = query.record().count();// query.size();
            QString str = query.value(valIdx).toString();

            if (!caseCompare)
                str = str.toUpper();

            if (exceptsSet.find(str)!=exceptsSet.end())
                continue;

            resVec.push_back(str);
            
        } while(query.next());

        return resVec;

    }

    //------------------------------
    virtual QVector<QString> queryToSingleStringVector( QSqlQuery& query, int valIdx, const QStringList      &except, bool caseCompare ) const override
    {
        QVector<QString> vec;
        for (QStringList::const_iterator it = except.constBegin(); it != except.constEnd(); ++it)
        {
            #if defined(_DEBUG) || defined(DEBUG)
            QString curStr = *it;
            #endif
            vec.push_back(*it);
        }

        return queryToSingleStringVector( query, valIdx, vec, caseCompare );
    }

    //------------------------------
    virtual QVector<QString> queryToSingleStringVector( QSqlQuery& query, int valIdx,       QString           except, bool caseCompare ) const override
    {
        except.replace(':', ";");
        except.replace(',', ";");
        except.replace('.', ";");
        return queryToSingleStringVector( query, valIdx, except.split( ';', Qt::SkipEmptyParts ), caseCompare );
    }

    //------------------------------
    virtual QVector<QString> queryToSingleStringVector( QSqlQuery& query, int valIdx ) const override
    {
        return queryToSingleStringVector( query, valIdx, QVector<QString>(), false );
    }

    //------------------------------
    
    
    
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

