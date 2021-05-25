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
class UserDatabaseManagerSQLiteImpl : public DatabaseManagerSQLiteImplBase
{

protected:

    //------------------------------
    friend QSharedPointer<IDatabaseManager> createUserDatabaseManager( QSharedPointer<QSqlDatabase> pDb, QSharedPointer<DatabaseConfig> pDatabaseConfig, QSharedPointer<LoggingConfig> pLoggingConfig );


    //------------------------------
    UserDatabaseManagerSQLiteImpl( QSharedPointer<QSqlDatabase> pDb, QSharedPointer<DatabaseConfig> pDatabaseConfig, QSharedPointer<LoggingConfig> pLoggingConfig )
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

            tablesLevel_0.insert("TRADING_TARIFF_HISTORY");
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

        //m["INSTRUMENT_CANDLES"] = QVector<QString>{ "INSTRUMENT_ID,STOCK_EXCHANGE_ID,CANDLE_RESOLUTION_ID,CANDLE_DATE_TIME" };

        return m;
    }



    virtual const QMap<QString,QString>& getTableSchemas() const override
    {
        static bool schemasInitialized = false;
        
        static QMap<QString,QString> tableSchemas = modelMakeAllSqlSchemas_SQLITE();

        if (!schemasInitialized)
        {
            schemasInitialized = true;

            tableSchemas["TRADING_TARIFF_HISTORY"] = "TRADING_TARIFF_ID  INTEGER REFERENCES TRADING_TARIFF,"      + lf() +
                                                     "BROKER_ACCOUNT_TYPE_ID INTEGER REFERENCES BROKER_ACCOUNT_TYPE," + lf() +
                                                     "SINCE_DATE_TIME       VARCHAR(24) NOT NULL,"                   + lf() +
                                                     "COMMISSION            DECIMAL(18,8)"
                                                      ;

            tableSchemas["OPERATIONS"         ] = "ID                    VARCHAR(32) NOT NULL UNIQUE,"            + lf() +
                                                  "BROKER_ACCOUNT_TYPE_ID INTEGER REFERENCES BROKER_ACCOUNT_TYPE," + lf() +
                                                  "OPERATION_TYPE_ID     INTEGER REFERENCES OPERATION_TYPE_WITH_COMMISSION," + lf() +
                                                  "OPERATION_STATUS_ID   INTEGER REFERENCES OPERATION_STATUS,"    + lf() +
                                                  "STOCK_EXCHANGE_ID     INTEGER REFERENCES STOCK_EXCHANGE_LIST," + lf() +
                                                  "PAYMENT               DECIMAL(18,8),"                          + lf() +
                                                  "PAYMENT_CURRENCY_ID   INTEGER REFERENCES CURRENCY,"            + lf() +
                                                  "COMMISSION            DECIMAL(18,8),"                          + lf() +
                                                  "COMMISSION_CURRENCY_ID INTEGER REFERENCES CURRENCY,"           + lf() +
                                                  "QUANTITY              DECIMAL(18,8),"                          + lf() +
                                                  "QUANTITY_EXECUTED     DECIMAL(18,8)"
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
        

            initData["TRADING_TARIFF_HISTORY"] = "1,1,2021-02-14 00:00:00,0.3;"
                                                 "2,1,2021-03-27 00:00:00,0.05";


        } // if (initData.empty())


        std::map<QString, QString>::const_iterator it = initData.find(tableName);
        if (it!=initData.end())
        {
            return it->second;
        }

        // Relaxed init data creation

        return QString();
    }





}; // UserDatabaseManagerSQLiteImpl

//----------------------------------------------------------------------------




} // namespace invest_openapi

