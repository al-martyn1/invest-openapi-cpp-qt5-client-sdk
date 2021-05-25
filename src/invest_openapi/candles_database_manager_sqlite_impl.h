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
//#include "i_oa_database_manager.h"

#include "models.h"
#include "utility.h"

#include "model_to_strings.h"

#include "qt_time_helpers.h"

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
namespace invest_openapi
{



//----------------------------------------------------------------------------
class CandlesDatabaseManagerSQLiteImpl : public DatabaseManagerSQLiteImplBase
{

protected:

    //------------------------------
    friend QSharedPointer<IDatabaseManager> createCandlesDatabaseManager( QSharedPointer<QSqlDatabase> pDb, QSharedPointer<DatabaseConfig> pDatabaseConfig, QSharedPointer<LoggingConfig> pLoggingConfig );


    //------------------------------
    CandlesDatabaseManagerSQLiteImpl( QSharedPointer<QSqlDatabase> pDb, QSharedPointer<DatabaseConfig> pDatabaseConfig, QSharedPointer<LoggingConfig> pLoggingConfig )
    : DatabaseManagerSQLiteImplBase(pDb, pDatabaseConfig, pLoggingConfig)
    {}

    //----------------------------------------------------------------------------
    // IDatabaseManager


    //------------------------------
    virtual QSet<QString> tableGetTableNamesForCreation( int creationLevel ) const override
    {
        static bool levelsInitialized = false;

        static QSet<QString> tablesLevel_0;
        //static QSet<QString> tablesLevel_1;

        if (!levelsInitialized)
        {
            levelsInitialized = true;

            tablesLevel_0.insert("INSTRUMENT_CANDLES");
        }

        switch(creationLevel)
        {
            case 0 : return tablesLevel_0;
            default: return QSet<QString>();
        }

    }

    virtual const QMap<QString,QVector<QString> >& getAdditionalUniques() const override
    {
        static QMap<QString,QVector<QString> > m;
        if (!m.empty())
            return m;

        m["INSTRUMENT_CANDLES"] = QVector<QString>{ "INSTRUMENT_ID,STOCK_EXCHANGE_ID,CANDLE_RESOLUTION_ID,CANDLE_DATE_TIME" };

        return m;
    }



    virtual const QMap<QString,QString>& getTableSchemas() const override
    {
        static bool schemasInitialized = false;
        
        static QMap<QString,QString> tableSchemas = modelMakeAllSqlSchemas_SQLITE();

        if (!schemasInitialized)
        {
            schemasInitialized = true;

            tableSchemas["INSTRUMENT_CANDLES" ] = "INSTRUMENT_ID         INTEGER ,"   + lf() + // REFERENCES MARKET_INSTRUMENT
                                                  "STOCK_EXCHANGE_ID     INTEGER ,"   + lf() + // REFERENCES STOCK_EXCHANGE_LIST
                                                  "CANDLE_RESOLUTION_ID  INTEGER ,"   + lf() + // REFERENCES CANDLE_RESOLUTION
                                                  "CANDLE_DATE_TIME      VARCHAR(24) NOT NULL,"                   + lf() +
                                                  "CURRENCY_ID           INTEGER,"                                + lf() +
                                                  "OPEN_PRICE            DECIMAL(18,8) NOT NULL,"                 + lf() +
                                                  "CLOSE_PRICE           DECIMAL(18,8) NOT NULL,"                 + lf() +
                                                  "HIGH_PRICE            DECIMAL(18,8) NOT NULL,"                 + lf() +
                                                  "LOW_PRICE             DECIMAL(18,8) NOT NULL,"                 + lf() +
                                                  "VOLUME                DECIMAL(18,8) NOT NULL"
                                                ;

        }

        return tableSchemas;
    
    }


    //------------------------------
    //const QMap<QString,QString>& getTableInitialDataMap() const
    //QString getTableInitData( const QString &tableName ) const
    virtual QString getTableInitData( const QString &tableName ) const override
    {
        std::map<QString, QString> initData;

        if (initData.empty())
        {
        
        } // if (initData.empty())


        std::map<QString, QString>::const_iterator it = initData.find(tableName);
        if (it!=initData.end())
        {
            return it->second;
        }

        // Relaxed init data creation

        return QString();
    }





}; // CandlesDatabaseManagerSQLiteImpl

//----------------------------------------------------------------------------




} // namespace invest_openapi

