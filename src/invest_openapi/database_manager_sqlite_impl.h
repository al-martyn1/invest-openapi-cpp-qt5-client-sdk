#pragma once

#include <QObject>
#include <QVariant>
#include <QString>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlField>
#include <QSqlQuery>
#include <QMap>
#include <QSet>
#include <QSharedPointer>

#include <exception>
#include <stdexcept>


#include "database_manager_sqlite_impl_base.h"
#include "i_oa_database_manager.h"

#include "models.h"
#include "utility.h"

#include "model_to_strings.h"

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
namespace invest_openapi
{



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
        QVector<QStringPair>           pairs = simpleSplitTo<QStringPair>( all );
        QVector<QStringPair>::iterator it    = pairs.begin();

        for(; it != pairs.end(); ++it)
        {
            if (!insertNewCurrencyType( it->first, it->second ))
                return false;
        }
        return true;
    }

    virtual bool   insertNewInstrumentTypes  ( const QString &all ) const override
    {
        QVector<QStringPair>           pairs = simpleSplitTo<QStringPair>( all );
        QVector<QStringPair>::iterator it    = pairs.begin();

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
    virtual QVector<QString> tableGetColumnsFromDb( const QString &tableName ) const override
    {
        return tableGetColumnsFromDbInternal( tableMapName(tableName) );
    }

    //------------------------------
    virtual QSet<QString> tableGetTableNamesForCreation( int creationLevel ) const override
    {
        static bool levelsInitialized = false;

        static QSet<QString> tablesLevel_0;
        static QSet<QString> tablesLevel_1;
        static QSet<QString> tablesLevel_2;

        if (!levelsInitialized)
        {
            levelsInitialized = true;

            // Basic ref books
            tablesLevel_0.insert("BROKER_ACCOUNT_TYPE");
            tablesLevel_0.insert("CURRENCY");
            tablesLevel_0.insert("INSTRUMENT_TYPE");
            tablesLevel_0.insert("CANDLE_RESOLUTION");
            tablesLevel_0.insert("OPERATION_TYPE");
            tablesLevel_0.insert("ORDER_STATUS");
            tablesLevel_0.insert("ORDER_TYPE");

            tablesLevel_1.insert("MARKET_INSTRUMENT");

            tablesLevel_2.insert("TIMEZONE");
            tablesLevel_2.insert("STOCK_EXCHANGE_LIST");
            tablesLevel_2.insert("INSTRUMENT_LISTING_DATES");
            tablesLevel_2.insert("INSTRUMENT_CANDLES");

            

            /*
            tablesLevel_2.insert("_META_TABLES");
            tablesLevel_2.insert("_META_COLUMNS");
            */
        }

        switch(creationLevel)
        {
            case 0 : return tablesLevel_0;
            case 1 : return tablesLevel_1;
            case 2 : return tablesLevel_2;
            default: return QSet<QString>();
        }

    }

    const QMap<QString,QVector<QString> >& getAdditionalUniques() const
    {
        static QMap<QString,QVector<QString> > m;
        if (!m.empty())
            return m;

        m["INSTRUMENT_LISTING_DATES"] = QVector<QString>{ "INSTRUMENT_ID,STOCK_EXCHANGE_ID", "INSTRUMENT_FIGI,STOCK_EXCHANGE_NAME" };

        return m;
    }

    QVector<QString> tableGetAdditionalUniques  ( const QString &tableName    ) const override
    {
        const QMap<QString,QVector<QString> >& additionalUniques = getAdditionalUniques();

        auto it = additionalUniques.find(tableName);

        if (it==additionalUniques.end())
            return QVector<QString>();

        QVector<QString> uList = *it;

        QVector<QString> res;

        for( auto u : uList)
        {
            res.push_back( QString("UNIQUE(%1)").arg(u) );
        }

        return res;
        //return *it;

    }

    const QMap<QString,QString>& getTableSchemas() const
    {
        static bool schemasInitialized = false;
        
        static QMap<QString,QString> tableSchemas = modelMakeAllSqlSchemas_SQLITE();

        if (!schemasInitialized)
        {
            schemasInitialized = true;

            tableSchemas[QString("BROKER_ACCOUNT_TYPE")] = modelMakeSqlCreateTableSchema_SQLITE( modelMakeSqlSchemaStringVector_SQLITE<BrokerAccountType>(QString(), false ) );
            tableSchemas[QString("CURRENCY"           )] = modelMakeSqlCreateTableSchema_SQLITE( modelMakeSqlSchemaStringVector_SQLITE<Currency         >(QString(), false ) );
            tableSchemas[QString("INSTRUMENT_TYPE"    )] = modelMakeSqlCreateTableSchema_SQLITE( modelMakeSqlSchemaStringVector_SQLITE<InstrumentType   >(QString(), false ) );
            tableSchemas[QString("CANDLE_RESOLUTION"  )] = modelMakeSqlCreateTableSchema_SQLITE( modelMakeSqlSchemaStringVector_SQLITE<CandleResolution >(QString(), false ) );
            tableSchemas[QString("OPERATION_TYPE"     )] = modelMakeSqlCreateTableSchema_SQLITE( modelMakeSqlSchemaStringVector_SQLITE<OperationType    >(QString(), false ) );
            tableSchemas[QString("ORDER_STATUS"       )] = modelMakeSqlCreateTableSchema_SQLITE( modelMakeSqlSchemaStringVector_SQLITE<OrderStatus      >(QString(), false ) );
            tableSchemas[QString("ORDER_TYPE"         )] = modelMakeSqlCreateTableSchema_SQLITE( modelMakeSqlSchemaStringVector_SQLITE<OrderType        >(QString(), false ) );

            tableSchemas[QString("TIMEZONE"           )] = "ID               INTEGER PRIMARY KEY AUTOINCREMENT,"
                                                           "NAME             VARCHAR(64) NOT NULL UNIQUE,"
                                                           "DESCRIPTION      VARCHAR(255)"
                                                         ;

            tableSchemas[QString("STOCK_EXCHANGE_LIST")] = "ID               INTEGER PRIMARY KEY AUTOINCREMENT,"
                                                           "NAME             VARCHAR(32) NOT NULL UNIQUE,"
                                                           "FOUNDATION_DATE  VARCHAR(10),"
                                                           "TIMEZONE_ID      INTEGER REFERENCES TIMEZONE,"
                                                           "TIMEZONE_NAME    VARCHAR(64),"
                                                           "DESCRIPTION      VARCHAR(255)"
                                                         ;

            tableSchemas[QString("INSTRUMENT_LISTING_DATES")] = "INSTRUMENT_ID         INTEGER REFERENCES MARKET_INSTRUMENT,"
                                                           "INSTRUMENT_FIGI       VARCHAR(12) NOT NULL,"
                                                           "INSTRUMENT_TICKER     VARCHAR(12) NOT NULL,"
                                                           "STOCK_EXCHANGE_ID     INTEGER REFERENCES STOCK_EXCHANGE_LIST,"
                                                           "STOCK_EXCHANGE_NAME   VARCHAR(32),"
                                                           "LISTING_DATE          VARCHAR(10)"
                                                         ;

            tableSchemas[QString("INSTRUMENT_CANDLES" )] = "INSTRUMENT_ID         INTEGER REFERENCES MARKET_INSTRUMENT,"
                                                           "INSTRUMENT_FIGI       VARCHAR(12) NOT NULL,"
                                                           "INSTRUMENT_TICKER     VARCHAR(12) NOT NULL,"
                                                           "STOCK_EXCHANGE_ID     INTEGER REFERENCES STOCK_EXCHANGE_LIST NOT NULL,"
                                                           "STOCK_EXCHANGE_NAME   VARCHAR(32) NOT NULL,"
                                                           "CANDLE_RESOLUTION_ID  INTEGER REFERENCES CANDLE_RESOLUTION NOT NULL,"
                                                           "CANDLE_RESOLUTION     VARCHAR(8) NOT NULL,"
                                                           "CANDLE_DATE_TIME      VARCHAR(24) NOT NULL,"
                                                           "OPEN_PRICE            DECIMAL(18,8) NOT NULL,"
                                                           "CLOSE_PRICE           DECIMAL(18,8) NOT NULL,"
                                                           "HIGH_PRICE            DECIMAL(18,8) NOT NULL,"
                                                           "LOW_PRICE             DECIMAL(18,8) NOT NULL,"
                                                           "VOLUME                DECIMAL(18,8) NOT NULL"
                                                         ;


            // ISO8601 YYYY-MM-DD HH:MM:SS.SSS
            // YYYY-MM-DD               - 4-2-2       - 4+1+2+1+2      - 10 chars
            // HH:MM:SS.SSS             - 2:2:2.3     - 2+1+2+1+2+1+3  - 12 chars
            // YYYY-MM-DD HH:MM:SS.SSS  - 10 [SP] 12  - 10+1+12        - 23 chars
            //
            /*
            tableSchemas[QString("STOCK_EXCHANGE_LIST")] = "ID               INTEGER PRIMARY KEY AUTOINCREMENT,"
                                                           "NAME             VARCHAR(12) NOT NULL UNIQUE,"
                                                           "FOUNDATION_DATE"

            */
            //yyyy-MM-dd


            // Moskovskaya Birzha MMVB-RTS PAO
            // Moscow Exchange
            // Moscow Exchange was established on 19 December 2011 by merging the two largest Moscow-based exchanges, the Moscow Interbank Currency Exchange (MICEX) and the Russian Trading System (RTS)
            //tableSchemas[QString("STOCK_EXCHANGE_LIST"         )] = 

            /*
            tableSchemas[QString("_META_TABLES"       )] = lf()    + QString("TABLE_NAME")            + tab() + QString("VARCHAR(64) NOT NULL UNIQUE") 
                                                         + lf(',') + QString("DISPLAY_NAME")          + tab() + QString("TEXT")
                                                         + lf(',') + QString("DESCRIPTION")           + tab() + QString("TEXT")
                                                         ;

            tableSchemas[QString("_META_COLUMNS"      )] = lf()    + QString("TABLE_NAME")            + tab() + QString("VARCHAR(64) NOT NULL UNIQUE") 
                                                         + lf(',') + QString("COLUMN_NAME")           + tab() + QString("VARCHAR(64) NOT NULL UNIQUE") 
                                                         + lf(',') + QString("DISPLAY_NAME")          + tab() + QString("TEXT")
                                                         + lf(',') + QString("DESCRIPTION")           + tab() + QString("TEXT")
                                                         ;
            */
            //tableSchemas[QString("")] = 

        }

        return tableSchemas;
    
    }

    virtual QVector<QString> tableGetColumnsFromSchema  ( const QString &tableName ) const override
    {
        static QMap<QString, QVector<QString> > tableColumns;

        if (tableColumns.empty())
        {
            const QMap<QString,QString>& tableSchemas = getTableSchemas();
            QList<QString> keys = tableSchemas.keys();

            for( const auto &k : keys)
            {
                tableColumns[k] = getColumnNamesFromTableSqlSchema(tableSchemas[k]);
            } // for

        } // if

        auto it = tableColumns.find(tableName);

        if (it==tableColumns.end())
            return QVector<QString>();

        return *it;

    }
    

    virtual QString tableGetSchema      ( const QString &tableName  ) const override
    {
        // tableName = tableMapName(tableName)

        // https://sqlite.org/lang_createtable.html
        // https://www.tutorialspoint.com/sqlite/sqlite_data_types.htm
        // https://sqlite.org/datatype3.html
        // https://www.programmersought.com/article/1613993309/

        const QMap<QString,QString>& tableSchemas = getTableSchemas();

        auto it = tableSchemas.find(tableName);

        if (it==tableSchemas.end())
            return QString();

        return *it;

        /*
        if (tableName.toUpper()=="INSTRUMENTS")
        {
            return lf()    + QString("ID")                  + tab() + QString("INTEGER NOT NULL UNIQUE") 
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
                 + lf(',') + QString("PRIMARY KEY(ID AUTOINCREMENT)")
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
            return lf()    + QString("ID")                    + tab() + QString("INTEGER NOT NULL UNIQUE") 
                 + lf(',') + QString("INSTRUMENT_TYPE")       + tab() + QString("VARCHAR(8) NOT NULL UNIQUE")
                 + lf(',') + QString("DESCRIPTION")           + tab() + QString("TEXT")
                 + lf(',') + QString("PRIMARY KEY(ID)")
                 ;
        }
        else if (tableName.toUpper()=="_META_TABLES")
        {
            return lf()    + QString("TABLE_NAME")            + tab() + QString("VARCHAR(64) NOT NULL UNIQUE") 
                 + lf(',') + QString("DISPLAY_NAME")          + tab() + QString("TEXT")
                 + lf(',') + QString("DESCRIPTION")           + tab() + QString("TEXT")
                 ;
        }
        else if (tableName.toUpper()=="_META_COLUMNS")
        {
            return lf()    + QString("TABLE_NAME")            + tab() + QString("VARCHAR(64) NOT NULL UNIQUE") 
                 + lf(',') + QString("COLUMN_NAME")           + tab() + QString("VARCHAR(64) NOT NULL UNIQUE") 
                 + lf(',') + QString("DISPLAY_NAME")          + tab() + QString("TEXT")
                 + lf(',') + QString("DESCRIPTION")           + tab() + QString("TEXT")
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
        */

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

