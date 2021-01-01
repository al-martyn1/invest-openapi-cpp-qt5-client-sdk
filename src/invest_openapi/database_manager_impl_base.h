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


#include "i_database_manager.h"

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
namespace invest_openapi
{



//----------------------------------------------------------------------------
class DatabaseManagerImplBase : public IDatabaseManager
{

protected:

    //------------------------------
    DatabaseManagerImplBase( QSharedPointer<QSqlDatabase> pDb, const DatabaseConfig &databaseConfig )
    : m_databaseConfig(databaseConfig)
    , m_pDb(pDb)
    {}

    //------------------------------
    DatabaseManagerImplBase();
    DatabaseManagerImplBase( const DatabaseManagerImplBase &);
    DatabaseManagerImplBase& operator=( const DatabaseManagerImplBase &);


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
            return m_databaseConfig.tableNameInstruments;
        }

        //checkTableName(QString());
        //return QString();
        return tableName; // return unmapped value
    }

    //------------------------------
    DatabaseConfig                    m_databaseConfig;
    QSharedPointer<QSqlDatabase>      m_pDb;

    unsigned                          m_defDecimalTotal = 0;
    unsigned                          m_defDecimalFrac  = 0;

    //------------------------------

}; // class DatabaseManagerImplBase

//----------------------------------------------------------------------------




} // namespace invest_openapi

