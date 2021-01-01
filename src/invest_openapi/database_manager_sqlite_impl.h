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
    friend QSharedPointer<IDatabaseManager> createDatabaseManager( QSharedPointer<QSqlDatabase> pDb, QSharedPointer<DatabaseConfig> pDatabaseConfig, QSharedPointer<LoggingConfig> pLoggingConfig );


    //------------------------------
    DatabaseManagerSQLiteImpl( QSharedPointer<QSqlDatabase> pDb, QSharedPointer<DatabaseConfig> pDatabaseConfig, QSharedPointer<LoggingConfig> pLoggingConfig )
    : DatabaseManagerSQLiteImplBase(pDb, pDatabaseConfig, pLoggingConfig)
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
        = QString( "INSERT INTO " ) + tableMapName("CURRENCIES")
        + QString("\r\n(ID, CURRENCY, DESCRIPTION)")
        + QString("\r\nVALUES(\r\n")
        + tab() + q( toQString(toInt(c)) )
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
        + QString("\r\n(ID, INSTRUMENT_TYPE, DESCRIPTION)")
        + QString("\r\nVALUES(\r\n")
        + tab() + q( toQString(toInt(t)) )
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
        QList<QStringPair>           pairs = simpleSplitTo<QStringPair>( all );
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
        QList<QStringPair>           pairs = simpleSplitTo<QStringPair>( all );
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
            return lf()    + QString("INSTRUMENT_ID")       + tab() + QString("INTEGER NOT NULL UNIQUE") 
                 + lf(',') + QString("FIGI")                + tab() + QString("VARCHAR(12) NOT NULL UNIQUE")
                 + lf(',') + QString("ISIN")                + tab() + QString("VARCHAR(12) UNIQUE")
                 + lf(',') + QString("TICKER")              + tab() + QString("VARCHAR(12) NOT NULL UNIQUE")
                 + lf(',') + QString("PROCE_INCREMENT")     + tab() + defDecimal()
                 + lf(',') + QString("FACE_VALUE")          + tab() + defDecimal()
                 + lf(',') + QString("LOT_SIZE_LIMIT")      + tab() + QString("INTEGER") 
                 + lf(',') + QString("LOT_SIZE_MARKET")     + tab() + QString("INTEGER") 
                 + lf(',') + QString("MIN_QUANTITY")        + tab() + QString("INTEGER") 
                 + lf(',') + QString("CURRENCY_ID")         + tab() + QString("INTEGER") 
                 + lf(',') + QString("CURRENCY")            + tab() + QString("VARCHAR(4)")
                 + lf(',') + QString("INSTRUMENT_TYPE_ID")  + tab() + QString("INTEGER") 
                 + lf(',') + QString("INSTRUMENT_TYPE")     + tab() + QString("VARCHAR(4)")
                 + lf(',') + QString("NAME")                + tab() + QString("TEXT")
                 + lf(',') + QString("PRIMARY KEY(INSTRUMENT_ID AUTOINCREMENT)")
                 + lf(',') + QString("FOREIGN KEY(CURRENCY_ID) REFERENCES CURRENCIES(ID)")
                 + lf(',') + QString("FOREIGN KEY(INSTRUMENT_TYPE_ID) REFERENCES INSTRUMENT_TYPES(ID)")
                 ;
        }
        else if (tableName.toUpper()=="CURRENCIES")
        {
            // CURRENCY - RUB/USD/EUR/GBP/HKD/CHF/JPY/CNY/TRY
            return lf()    + QString("ID")                  + tab() + QString("INTEGER NOT NULL UNIQUE") 
                 + lf(',') + QString("CURRENCY")            + tab() + QString("VARCHAR(4) NOT NULL UNIQUE")
                 + lf(',') + QString("DESCRIPTION")         + tab() + QString("TEXT")
                 + lf(',') + QString("PRIMARY KEY(ID)")
                 ;
        }
        else if (tableName.toUpper()=="INSTRUMENT_TYPES")
        {
            // CURRENCY - RUB/USD/EUR/GBP/HKD/CHF/JPY/CNY/TRY
            return lf()    + QString("ID")                    + tab() + QString("INTEGER NOT NULL UNIQUE") 
                 + lf(',') + QString("INSTRUMENT_TYPE")       + tab() + QString("VARCHAR(8) NOT NULL UNIQUE")
                 + lf(',') + QString("DESCRIPTION")           + tab() + QString("TEXT")
                 + lf(',') + QString("PRIMARY KEY(ID)")
                 ;
        }
        else if (tableName.toUpper()=="TEST")
        {
            return lf()    + QString("DECIMAL_12_3")        + tab() + QString("DECIMAL(12,3)") 
                 + lf(',') + QString("DECIMAL_18_6_NN")     + tab() + QString("DECIMAL(18,6) NOT NULL")
                 + lf(',') + QString("CHAR_12")             + tab() + QString("CHAR(12)")
                 + lf(',') + QString("CHAR_12_NN")          + tab() + QString("CHAR(12) NOT NULL")
                 + lf(',') + QString("DEF_DEC_111_1")       + tab() + defDecimal(111, 1)
                 + lf(',') + QString("DEF_DEC")             + tab() + defDecimal()
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

