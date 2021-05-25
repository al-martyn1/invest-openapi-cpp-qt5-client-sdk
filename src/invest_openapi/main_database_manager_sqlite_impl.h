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
class MainDatabaseManagerSQLiteImpl : public DatabaseManagerSQLiteImplBase
{

protected:

    //------------------------------
    friend QSharedPointer<IDatabaseManager> createMainDatabaseManager( QSharedPointer<QSqlDatabase> pDb, QSharedPointer<DatabaseConfig> pDatabaseConfig, QSharedPointer<LoggingConfig> pLoggingConfig );


    //------------------------------
    MainDatabaseManagerSQLiteImpl( QSharedPointer<QSqlDatabase> pDb, QSharedPointer<DatabaseConfig> pDatabaseConfig, QSharedPointer<LoggingConfig> pLoggingConfig )
    : DatabaseManagerSQLiteImplBase(pDb, pDatabaseConfig, pLoggingConfig)
    {}

    //----------------------------------------------------------------------------
    // IDatabaseManager


    //------------------------------
    virtual QSet<QString> tableGetTableNamesForCreation( int creationLevel ) const override
    {
        static bool levelsInitialized = false;

        static QSet<QString> tablesLevel_0;
        static QSet<QString> tablesLevel_1;
        static QSet<QString> tablesLevel_2;
        static QSet<QString> tablesLevel_3;
        static QSet<QString> tablesLevel_4;

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
            tablesLevel_0.insert("OPERATION_STATUS");
            tablesLevel_0.insert("OPERATION_TYPE_WITH_COMMISSION");
            tablesLevel_0.insert("TRADING_TARIFF");

            tablesLevel_1.insert("MARKET_INSTRUMENT");
            //tablesLevel_1.insert("TRADING_TARIFF_HISTORY");

            tablesLevel_2.insert("TIMEZONE");
            tablesLevel_2.insert("STOCK_EXCHANGE_LIST");
            tablesLevel_2.insert("INSTRUMENT_LISTING_DATES");
            //tablesLevel_2.insert("INSTRUMENT_CANDLES");
            tablesLevel_3.insert("INSTRUMENT_STATUS");

            //tablesLevel_3.insert("OPERATIONS");
            
            //tablesLevel_4.insert("OPERATION_TRADE");
            


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
            case 3 : return tablesLevel_3;
            case 4 : return tablesLevel_4;
            default: return QSet<QString>();
        }

    }

    virtual const QMap<QString,QVector<QString> >& getAdditionalUniques() const override
    {
        static QMap<QString,QVector<QString> > m;
        if (!m.empty())
            return m;

        m["INSTRUMENT_LISTING_DATES"] = QVector<QString>{ "INSTRUMENT_ID,STOCK_EXCHANGE_ID", "INSTRUMENT_FIGI,STOCK_EXCHANGE_NAME" };
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

            tableSchemas["BROKER_ACCOUNT_TYPE"] = modelMakeSqlCreateTableSchema_SQLITE( modelMakeSqlSchemaStringVector_SQLITE<BrokerAccountType>(QString(), false ) );
            tableSchemas["CURRENCY"           ] = modelMakeSqlCreateTableSchema_SQLITE( modelMakeSqlSchemaStringVector_SQLITE<Currency         >(QString(), false ) );
            tableSchemas["INSTRUMENT_TYPE"    ] = modelMakeSqlCreateTableSchema_SQLITE( modelMakeSqlSchemaStringVector_SQLITE<InstrumentType   >(QString(), false ) );
            tableSchemas["CANDLE_RESOLUTION"  ] = modelMakeSqlCreateTableSchema_SQLITE( modelMakeSqlSchemaStringVector_SQLITE<CandleResolution >(QString(), false ) );
            tableSchemas["OPERATION_TYPE"     ] = modelMakeSqlCreateTableSchema_SQLITE( modelMakeSqlSchemaStringVector_SQLITE<OperationType    >(QString(), false ) );
            tableSchemas["ORDER_STATUS"       ] = modelMakeSqlCreateTableSchema_SQLITE( modelMakeSqlSchemaStringVector_SQLITE<OrderStatus      >(QString(), false ) );
            tableSchemas["ORDER_TYPE"         ] = modelMakeSqlCreateTableSchema_SQLITE( modelMakeSqlSchemaStringVector_SQLITE<OrderType        >(QString(), false ) );
            tableSchemas["OPERATION_STATUS"   ] = modelMakeSqlCreateTableSchema_SQLITE( modelMakeSqlSchemaStringVector_SQLITE<OperationStatus   >(QString(), false ) );
            tableSchemas["OPERATION_TYPE_WITH_COMMISSION"] = modelMakeSqlCreateTableSchema_SQLITE( modelMakeSqlSchemaStringVector_SQLITE<OperationTypeWithCommission>(QString(), false ) );


            tableSchemas["TIMEZONE"           ] = //"ID               INTEGER PRIMARY KEY AUTOINCREMENT," + lf() +
                                                  "NAME             VARCHAR(64) NOT NULL UNIQUE,"       + lf() +
                                                  "DESCRIPTION      VARCHAR(255)"
                                                ;

            tableSchemas["STOCK_EXCHANGE_LIST"] = "ID               INTEGER PRIMARY KEY AUTOINCREMENT," + lf() +
                                                  "NAME             VARCHAR(32) NOT NULL UNIQUE,"       + lf() +
                                                  "FOUNDATION_DATE  VARCHAR(10),"                       + lf() +
                                                  "TIMEZONE         VARCHAR(64),"   + lf() + // REFERENCES TIMEZONE
                                                  // "TIMEZONE_ID      INTEGER REFERENCES TIMEZONE,"       + lf() +
                                                  // "TIMEZONE_NAME    VARCHAR(64),"                       + lf() +
                                                  "DESCRIPTION      VARCHAR(255)"
                                                ;

            tableSchemas["INSTRUMENT_LISTING_DATES"] = "INSTRUMENT_ID         INTEGER REFERENCES MARKET_INSTRUMENT," + lf() +
                                                       "INSTRUMENT_FIGI       VARCHAR(12) NOT NULL,"                      + lf() +
                                                       "INSTRUMENT_TICKER     VARCHAR(12) NOT NULL,"                      + lf() +
                                                       "STOCK_EXCHANGE_ID     INTEGER REFERENCES STOCK_EXCHANGE_LIST,"    + lf() +
                                                       "STOCK_EXCHANGE_NAME   VARCHAR(32),"                               + lf() +
                                                       "LISTING_DATE          VARCHAR(10)"
                                                     ;

            /*
            tableSchemas["INSTRUMENT_CANDLES" ] = "INSTRUMENT_ID         INTEGER REFERENCES MARKET_INSTRUMENT,"   + lf() +
                                                  "STOCK_EXCHANGE_ID     INTEGER REFERENCES STOCK_EXCHANGE_LIST," + lf() +
                                                  "CANDLE_RESOLUTION_ID  INTEGER REFERENCES CANDLE_RESOLUTION,"   + lf() +
                                                  "CANDLE_DATE_TIME      VARCHAR(24) NOT NULL,"                   + lf() +
                                                  "CURRENCY_ID           INTEGER REFERENCES CURRENCY,"            + lf() +
                                                  "OPEN_PRICE            DECIMAL(18,8) NOT NULL,"                 + lf() +
                                                  "CLOSE_PRICE           DECIMAL(18,8) NOT NULL,"                 + lf() +
                                                  "HIGH_PRICE            DECIMAL(18,8) NOT NULL,"                 + lf() +
                                                  "LOW_PRICE             DECIMAL(18,8) NOT NULL,"                 + lf() +
                                                  "VOLUME                DECIMAL(18,8) NOT NULL"
                                                ;
            */

            /*
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
            */

            tableSchemas["TRADING_TARIFF"     ] = "ID                    INTEGER PRIMARY KEY AUTOINCREMENT,"      + lf() +
                                                  "NAME                  VARCHAR(64) NOT NULL UNIQUE,"            + lf() +
                                                  "COMMISSION            DECIMAL(18,8),"                          + lf() +
                                                  "DESCRIPTION           VARCHAR(255)"
                                                  ;


            /*
            tableSchemas["TRADING_TARIFF_HISTORY"] = "TRADING_TARIFF_ID  INTEGER REFERENCES TRADING_TARIFF,"      + lf() +
                                                     "BROKER_ACCOUNT_TYPE_ID INTEGER REFERENCES BROKER_ACCOUNT_TYPE," + lf() +
                                                     "SINCE_DATE_TIME       VARCHAR(24) NOT NULL,"                   + lf() +
                                                     "COMMISSION            DECIMAL(18,8)"
                                                      ;
            */
            tableSchemas["INSTRUMENT_STATUS"  ] = "ID                    INTEGER PRIMARY KEY,"                    + lf() +
                                                  "STATUS                VARCHAR(10) NOT NULL,"                   + lf() +
                                                  "DESCRIPTION           VARCHAR(255) NOT NULL"                   + lf()
                                                  ;


            tableSchemas["INSTRUMENT_STATUS_LOG"] = "INSTRUMENT_ID         INTEGER REFERENCES MARKET_INSTRUMENT," + lf() +
                                                  "STATUS_ID               INTEGER REFERENCES INSTRUMENT_STATUS," + lf() +
                                                  "STOCK_EXCHANGE_ID       INTEGER REFERENCES STOCK_EXCHANGE_LIST," + lf() +
                                                  "SINCE_DATE_TIME         VARCHAR(24) NOT NULL"                  + lf()
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
        
            initData["INSTRUMENT_STATUS"]   = "0,INVALID,Invalid instrument status value;"
                                              "1,NORMAL,Instrument is in normal condition;"
                                              "2,DELISTED,Instrument is delisted - trading with this instrument has been terminated;"
                                              "3,ERRONEOUS,Instrument is erroneous - requests for this instrument will fails";

            initData["BROKER_ACCOUNT_TYPE"] = "0,INVALID,Invalid BrokerAccountType value;"
                                              "1,TINKOFF,Tinkoff broker account;"
                                              "2,TINKOFFIIS,Tinkoff IIS account";
           
            initData["CURRENCY"           ] = "0,INVALID,Invalid Currency value;"
                                              "1,RUB,Russian Ruble;"
                                              "2,USD,US Dollar;"
                                              "3,EUR,European Euro;"
                                              "4,GBP,Great Britain Pound Sterling;"
                                              "5,HKD,Hong Kong Dollar;"
                                              "6,CHF,Swiss Franc;"
                                              "7,JPY,Japanese Yen;"
                                              "8,CNY,Chinese Yuan;"
                                              "9,TRY,Turkish Lira";
           
            initData["INSTRUMENT_TYPE"    ] = "0,INVALID,Invalid InstrumentType value;"
                                              "1,STOCK,Stocks;"
                                              "2,CURRENCY,Currencies;"
                                              "3,BOND,Bonds;"
                                              "4,ETF,Etfs";
           
            // ID, RESOLUTION, INTERVAL_MIN, INTERVAL_MAX, INTERVAL_RECOMENDED_INCREMENT, DESCRIPTION
            initData["CANDLE_RESOLUTION"  ] = "0,INVALID,INVALID,INVALID,INVALID,Invalid CandleResolution value;"
                                              "1,1MIN,1MIN,1DAY,1DAY,1 min;"                        /* 1min [1 minute, 1 day]    */
                                              "2,2MIN,2MIN,1DAY,1DAY,2 min;"                        /* 2min [2 minutes, 1 day]   */
                                              "3,3MIN,3MIN,1DAY,1DAY,3 min;"                        /* 3min [3 minutes, 1 day]   */
                                              "4,5MIN,5MIN,1DAY,1DAY,5 min;"                        /* 5min [5 minutes, 1 day]   */
                                              "5,10MIN,10MIN,1DAY,1DAY,10 min;"                     /* 10min [10 minutes, 1 day] */
                                              "6,15MIN,15MIN,1DAY,1DAY,15 min;"                     /* 15min [15 minutes, 1 day] */
                                              "7,30MIN,30MIN,1DAY,1DAY,30 min;"                     /* 30min [30 minutes, 1 day] */
                                              "8,HOUR,1HOUR,7DAY,7DAY,Hour (60 min);"               /* hour [1 hour, 7 days]     */
                                              "9,DAY,1DAY,1YEAR,1YEAR,Day (1440 min);"               /* day [1 day, 1 year]       */
                                              "10,WEEK,7DAY,2YEAR,1YEAR,Week (10080 min);"           /* week [7 days, 2 years]    */
                                              "11,MONTH,1MONTH,10YEAR,5YEAR,Month (Avg 43200 min)";  /* month [1 month, 10 years] */
           
            initData["OPERATION_TYPE"     ] = "0,INVALID,Invalid OperationType value;"
                                              "1,BUY,Purchaise;"
                                              "2,SELL,Sell";
           
            initData["ORDER_STATUS"       ] = "0,INVALID,Invalid OrderStatus value;"
                                              "1,NEW,New order;"
                                              "2,PARTIALLYFILL,Partially fill;"
                                              "3,FILL,Fill;"
                                              "4,CANCELLED,Cancelled;"
                                              "5,REPLACED,Replaced;"
                                              "6,PENDINGCANCEL,Pending cancel;"
                                              "7,REJECTED,Rejected;"
                                              "8,PENDINGREPLACE,Pending replace;"
                                              "9,PENDINGNEW,Pending new";
           
            initData["ORDER_TYPE"         ] = "0,INVALID,Invalid OrderType value;"
                                              "1,LIMIT,Limit;"
                                              "2,MARKET,Market";
           
            initData["OPERATION_STATUS"   ] = "0,INVALID,Invalid OperationStatus value;"
                                              "1,DONE,Operation completed successfully;"
                                              "2,DECLINE,Operation declined;"
                                              "3,PROGRESS,Operation in progress";
           
            initData["OPERATION_TYPE_WITH_COMMISSION"] = "0,INVALID,Invalid OperationTypeWithCommission value;"
                                              "1,BUY,Buy;"
                                              "2,BUYCARD,Buycard;"
                                              "3,SELL,Sell;"
                                              "4,BROKERCOMMISSION,Broker commission;"
                                              "5,EXCHANGECOMMISSION,Exchange commission;"
                                              "6,SERVICECOMMISSION,Service commission;"
                                              "7,MARGINCOMMISSION,Margin commission;"
                                              "8,OTHERCOMMISSION,Other commission;"
                                              "9,PAYIN,Payin;"
                                              "10,PAYOUT,Payout;"
                                              "11,TAX,Tax;"
                                              "12,TAXLUCRE,Tax lucre;"
                                              "13,TAXDIVIDEND,Tax dividend;"
                                              "14,TAXCOUPON,Tax coupon;"
                                              "15,TAXBACK,Tax back;"
                                              "16,REPAYMENT,Repayment;"
                                              "17,PARTREPAYMENT,Part repayment;"
                                              "18,COUPON,Coupon;"
                                              "19,DIVIDEND,Dividend;"
                                              "20,SECURITYIN,Security in;"
                                              "21,SECURITYOUT,Securityout";
       
            initData["STOCK_EXCHANGE_LIST"] = "0,INVALID,,INVALID,Invalid stock exchange ID;"
                                              "1,MOEX,2011-12-19,MSK,PAO Moskovskaya Birzha";

            initData["TRADING_TARIFF"     ] = "0,INVALID,0.0,Invalid trading tariff;"
                                              "1,INVESTOR,0.3,Investor trading tariff;"
                                              "2,TRADER,0.05,Trader trading tariff - RUB 290 monthly fee;"
                                              "3,PREMIUM,0.025,Premium trading tariff - RUB 0/990/3000 monthly fee";

        } // if (initData.empty())


        std::map<QString, QString>::const_iterator it = initData.find(tableName);
        if (it!=initData.end())
        {
            return it->second;
        }

        // Relaxed init data creation

        if (tableName=="TIMEZONE")
        {
            std::vector<QString> dataLines;

            int tzKeyId = 0;

            // Create alias list
            std::vector<QString> tzAliasList = qt_helpers::getTimezonesAliasList<QString>();

            for( const auto &tzAlias : tzAliasList )
            {
                QString tzAliasDescription = qt_helpers::getTimezoneAliasDesciption(tzAlias);
                tzAliasDescription.replace(',', '/');
                //dataLines.push_back(QString("%1,%2,%3").arg(tzKeyId).arg(tzAlias).arg(tzAliasDescription));
                dataLines.push_back(QString("%1,%2").arg(tzAlias).arg(tzAliasDescription));
                ++tzKeyId;
            }


            // Create real timezones list

            QList<QByteArray> tzIdList = QTimeZone::availableTimeZoneIds();

            for( auto tzIdByteArray : tzIdList )
            {
                //std::string strTzId = tzId.toStdString();
                QString tzId = QString::fromStdString( tzIdByteArray.toStdString() );

                QTimeZone qtz     = QTimeZone(tzIdByteArray);
                QString   tzDescr = qtz.comment();
                tzDescr.replace(',', '/');
                //dataLines.push_back(QString("%1,%2,%3").arg(tzKeyId).arg(tzId).arg(tzDescr));
                dataLines.push_back(QString("%1,%2").arg(tzId).arg(tzDescr));
                ++tzKeyId;
            }

            QString resData = mergeString(dataLines, ";" );
            initData[tableName] = resData;

            return resData;
        
        } // if (tableName=="TIMEZONE")


        return QString();
    }





}; // MainDatabaseManagerSQLiteImpl

//----------------------------------------------------------------------------




} // namespace invest_openapi

