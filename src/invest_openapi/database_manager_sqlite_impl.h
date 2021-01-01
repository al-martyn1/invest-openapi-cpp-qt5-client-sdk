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


#include "database_manager_sqlite_impl_base.h"
#include "i_oa_database_manager.h"

#include "models.h"
#include "utility.h"

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
namespace invest_openapi
{


/*
struct IOaDatabaseManager
{
    virtual bool   insertNewCurrencyType     ( Currency       c, const QString & description ) const = 0;
    virtual bool   insertNewInstrumentType   ( InstrumentType t, const QString & description ) const = 0;

}; // struct IOaDatabaseManager

*/

//----------------------------------------------------------------------------
class DatabaseManagerSQLiteImpl : public DatabaseManagerSQLiteImplBase
                                , public IOaDatabaseManager
{

protected:

    //------------------------------
    friend QSharedPointer<IDatabaseManager> createDatabaseManager( QSharedPointer<QSqlDatabase> pDb, const DatabaseConfig &dbConfig );


    //------------------------------
    DatabaseManagerSQLiteImpl( QSharedPointer<QSqlDatabase> pDb, const DatabaseConfig &databaseConfig )
    : DatabaseManagerSQLiteImplBase(pDb, databaseConfig)
    {}

    //------------------------------
    bool exec( const QString & querySqlText, QSqlQuery &query ) const
    {
        RETURN_IOA_SQL_EXEC_QUERY( query, querySqlText );
    }

    //------------------------------
    bool exec( const QString & querySqlText ) const
    {
        QSqlQuery query(*m_pDb);
        return exec( querySqlText, query );
    }

    //----------------------------------------------------------------------------
    // IOaDatabaseManager methods

    //------------------------------
    virtual bool   insertNewCurrencyType     ( Currency         c, const QString & description ) const override
    {
        QString queryText
        = QString( "INSERT INTO " ) + tableMapName("CURRENCY_TYPES")
        + QString("\r\n(CURRENCY_ID, CURRENCY, DESCRIPTION)")
        + QString("\r\nVALUES(")
        + QString(" ") + q( toQString(toInt(c)) )
        + QString(",") + q( toQString(c) )
        + QString(",") + q( description )
        + QString("\r\n)") // end of values
        ;

        return exec(queryText);
    }

    //------------------------------
    virtual bool   insertNewInstrumentType   ( InstrumentType   t, const QString & description ) const override
    {
        QString queryText
        = QString( "INSERT INTO " ) + tableMapName("INSTRUMENT_TYPES")
        + QString("\r\n(INSTRUMENT_ID, INSTRUMENT, DESCRIPTION)")
        + QString("\r\nVALUES(")
        + QString(" ") + q( toQString(toInt(t)) )
        + QString(",") + q( toQString(t) )
        + QString(",") + q( description )
        + QString("\r\n)") // end of values
        ;

        return exec(queryText);
    }

    //------------------------------
    virtual bool   insertNewCurrencyType     ( const QString   &c, const QString & description ) const override
    {
        return insertNewCurrencyType( toCurrency(c), description );
    }

    //------------------------------
    virtual bool   insertNewInstrumentType   ( const QString   &t, const QString & description ) const override
    {
        return insertNewInstrumentType( toInstrumentType(t), description );
    }

    //----------------------------------------------------------------------------
    virtual bool   insertNewCurrencyTypes    ( const QString &all ) const override
    {
        QList<QStringPair>           pairs = simpleSplitToPairs( all );
        QList<QStringPair>::iterator it    = pairs.begin();

        for(; it != pairs.end(); ++it)
        {
            if (!insertNewCurrencyType( it->first, it->second ))
                return false;
        }
        return true;
    }

    virtual bool   insertNewInstrumentTypes  ( const QString &all ) const override
    {
        QList<QStringPair>           pairs = simpleSplitToPairs( all );
        QList<QStringPair>::iterator it    = pairs.begin();

        for(; it != pairs.end(); ++it)
        {
            if (!insertNewInstrumentType( it->first, it->second ))
                return false;
        }
        return true;
    }

    //----------------------------------------------------------------------------
    // IDatabaseManager

    //------------------------------
    virtual QString tableGetShema      ( QString tableName  ) const override
    {
        // tableName = tableMapName(tableName)

        // https://sqlite.org/lang_createtable.html
        // https://www.tutorialspoint.com/sqlite/sqlite_data_types.htm
        // https://sqlite.org/datatype3.html
        // https://www.programmersought.com/article/1613993309/

        if (tableName.toUpper()=="INSTRUMENTS")
        {
            return 
            QString(
            ""
            );
        }
        else if (tableName.toUpper()=="CURRENCY_TYPES")
        {
            // CURRENCY - RUB/USD/EUR/GBP/HKD/CHF/JPY/CNY/TRY
            return QString("\r\n ") + QString("CURRENCY_ID")         + QString(" ") + QString("INTEGER NOT NULL UNIQUE") 
                 + QString("\r\n,") + QString("CURRENCY")            + QString(" ") + QString("VARCHAR(4) NOT NULL UNIQUE")
                 + QString("\r\n,") + QString("DESCRIPTION")         + QString(" ") + QString("TEXT")
                 + QString("\r\n,") + QString("PRIMARY KEY(CURRENCY_ID)")
                 ;
        }
        else if (tableName.toUpper()=="INSTRUMENT_TYPES")
        {
            // CURRENCY - RUB/USD/EUR/GBP/HKD/CHF/JPY/CNY/TRY
            return QString("\r\n ") + QString("INSTRUMENT_ID")         + QString(" ") + QString("INTEGER NOT NULL UNIQUE") 
                 + QString("\r\n,") + QString("INSTRUMENT")            + QString(" ") + QString("VARCHAR(8) NOT NULL UNIQUE")
                 + QString("\r\n,") + QString("DESCRIPTION")           + QString(" ") + QString("TEXT")
                 + QString("\r\n,") + QString("PRIMARY KEY(INSTRUMENT_ID)")
                 ;
        }
        else if (tableName.toUpper()=="TEST")
        {
            return QString("\r\n ") + QString("DECIMAL_12_3")        + QString(" ") + QString("DECIMAL(12,3)") 
                 + QString("\r\n,") + QString("DECIMAL_18_6_NN")     + QString(" ") + QString("DECIMAL(18,6) NOT NULL")
                 + QString("\r\n,") + QString("CHAR_12")             + QString(" ") + QString("CHAR(12)")
                 + QString("\r\n,") + QString("CHAR_12_NN")          + QString(" ") + QString("CHAR(12) NOT NULL")
                 + QString("\r\n,") + QString("DEF_DEC_111_1")       + QString(" ") + defDecimal(111, 1)
                 + QString("\r\n,") + QString("DEF_DEC")             + QString(" ") + defDecimal()
                 //+ QString("")    +        QString("")        +      QString("")
                 ;
        }

        return QString();

        /*
        CREATE TABLE "TEST01" (
        "Field1" NUMERIC UNIQUE,
        "Field2" TEXT,
        "Field3" REAL NOT NULL,
        "Field4" BLOB,
        "Field5" INTEGER,
        PRIMARY KEY("Field5" AUTOINCREMENT)
        );
        */
        
        
        

    }

    //------------------------------


}; // DatabaseManagerSQLiteImpl

//----------------------------------------------------------------------------




} // namespace invest_openapi

