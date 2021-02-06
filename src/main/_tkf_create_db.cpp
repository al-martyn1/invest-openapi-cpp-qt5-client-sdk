/*! \file
    \brief Database creation exsample

 */

#include <iostream>
#include <exception>
#include <stdexcept>

#include <QCoreApplication>
#include <QString>
#include <QSettings>
#include <QTest>
#include <QDir>
#include <QElapsedTimer>

#include "invest_openapi/config_helpers.h"
#include "invest_openapi/api_config.h"
#include "invest_openapi/auth_config.h"
#include "invest_openapi/currencies_config.h"

#include "invest_openapi/invest_openapi.h"
#include "invest_openapi/factory.h"
#include "invest_openapi/openapi_completable_future.h"

#include "invest_openapi/database_config.h"
#include "invest_openapi/database_manager.h"

#include "cpp/cpp.h"


INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("_tkf_create_db");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;


    qDebug().nospace().noquote() << "Path to exe   : "<<QCoreApplication::applicationDirPath() /* .toStdString() */; // <<endl;
    //cout << endl;

    namespace tkf=invest_openapi;
    using tkf::config_helpers::lookupForConfigFile;
    using tkf::config_helpers::FileReadable;

    QString lookupConfSubfolders = "conf;config";

    auto dbConfigFullName  = lookupForConfigFile( "database.properties", lookupConfSubfolders, FileReadable() );
    auto logConfigFullName = lookupForConfigFile( "logging.properties" , lookupConfSubfolders, FileReadable() );

    qDebug().nospace().noquote() << "DB  Config : "<< dbConfigFullName ;
    qDebug().nospace().noquote() << "Log Config : "<< logConfigFullName ;


    QSharedPointer<tkf::DatabaseConfig> pDatabaseConfig = QSharedPointer<tkf::DatabaseConfig>( new tkf::DatabaseConfig(dbConfigFullName, tkf::DatabasePlacementStrategyDefault()) );
    QSharedPointer<tkf::LoggingConfig>  pLoggingConfig  = QSharedPointer<tkf::LoggingConfig> ( new tkf::LoggingConfig(logConfigFullName) );

    qDebug().nospace().noquote() << "DB name: " << pDatabaseConfig->dbFilename;

    QSharedPointer<QSqlDatabase> pSqlDb = QSharedPointer<QSqlDatabase>( new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE")) );
    pSqlDb->setDatabaseName( pDatabaseConfig->dbFilename );

    if (!pSqlDb->open())
    {
      qDebug() << pSqlDb->lastError().text();
      return 0;
    }

    QSharedPointer<tkf::IDatabaseManager> pDbMan = tkf::createDatabaseManager( pSqlDb, pDatabaseConfig, pLoggingConfig );

    //pDbMan->setDefDecimal(18,8);
    pDbMan->applyDefDecimalFromConfig( *pDatabaseConfig );

    int curLevel = 0;

    QSet<QString> tablesForCreation = pDbMan->tableGetTableNamesForCreation( curLevel );
    for(; !tablesForCreation.empty(); ++curLevel, tablesForCreation = pDbMan->tableGetTableNamesForCreation( curLevel ) )
    {
        qDebug().nospace().noquote() << "Level "<<curLevel;
        //cout<<endl<<
        //<<endl<<endl;
        
        for( auto tableName : tablesForCreation )
        {
            //cout<<"    "<<tableName.toStdString()<<endl;

            auto expandStrForTableName = cpp::makeExpandString( tableName.toStdString(), 20 );
            auto qexp = QString::fromStdString(expandStrForTableName);

            qDebug().nospace().noquote() << "Drop   table '" << tableName << "'" << qexp << ": " << pDbMan->tableDrop(tableName);
            qDebug().nospace().noquote() << "Create table '" << tableName << "'" << qexp << ": " << pDbMan->tableCreate(tableName);
            //qDebug().nospace().noquote() <<"\n";
            //qDebug().nospace().noquote() <<"";
        }
    }

    // Fill ref books here

    qDebug().nospace().noquote() << "Fill 'BROKER_ACCOUNT_TYPE' table: " 
                                 << pDbMan->insertToBulkFromString( "BROKER_ACCOUNT_TYPE"
                                                                  , "0,INVALID,Invalid BrokerAccountType value;"
                                                                    "1,TINKOFF,Tinkoff broker account;"
                                                                    "2,TINKOFFIIS,Tinkoff IIS account"
                                                                  );

    qDebug().nospace().noquote() << "Fill 'CURRENCY' table: " 
                                 << pDbMan->insertToBulkFromString( "CURRENCY"
                                                                  , "0,INVALID,Invalid Currency value;"
                                                                    "1,RUB,Russian Ruble;"
                                                                    "2,USD:US Dollar;"
                                                                    "3,EUR:European Euro;"
                                                                    "4,GBP:Great Britain Pound Sterling;"
                                                                    "5,HKD:Hong Kong Dollar;"
                                                                    "6,CHF:Swiss Franc;"
                                                                    "7,JPY:Japanese Yen;"
                                                                    "8,CNY:Chinese Yuan;"
                                                                    "9,TRY:Turkish Lira"
                                                                  );

    qDebug().nospace().noquote() << "Fill 'INSTRUMENT_TYPE' table: " 
                                 << pDbMan->insertToBulkFromString( "INSTRUMENT_TYPE"
                                                                  , "0,INVALID,Invalid InstrumentType value;"
                                                                    "1,STOCK,Stocks;"
                                                                    "2,CURRENCY,Currencies;"
                                                                    "3,BOND,Bonds;"
                                                                    "4,ETF,Etfs"
                                                                  );

    qDebug().nospace().noquote() << "Fill 'CANDLE_RESOLUTION' table: " 
                                 << pDbMan->insertToBulkFromString( "CANDLE_RESOLUTION"
                                                                  , "0,INVALID,Invalid CandleResolution value;"
                                                                    "1,_1MIN,1 min;"
                                                                    "2,_2MIN,2 min;"
                                                                    "3,_3MIN,3 min;"
                                                                    "4,_5MIN,5 min;"
                                                                    "5,_10MIN,10 min;"
                                                                    "6,_15MIN,15 min;"
                                                                    "7,_30MIN,30 min;"
                                                                    "8,HOUR,Hour (60 min);"
                                                                    "9,DAY,Day (1440 min);"
                                                                    "10,WEEK,Week (10080 min);"
                                                                    "11,MONTH,(Avg 43200 min)"
                                                                  );

    qDebug().nospace().noquote() << "Fill 'OPERATION_TYPE' table: " 
                                 << pDbMan->insertToBulkFromString( "OPERATION_TYPE"
                                                                  , "0,INVALID,Invalid OperationType value;"
                                                                    "1,BUY,Purchaise;"
                                                                    "2,SELL,Sell"
                                                                  );

    qDebug().nospace().noquote() << "Fill 'ORDER_STATUS' table: " 
                                 << pDbMan->insertToBulkFromString( "ORDER_STATUS"
                                                                  , "0,INVALID,Invalid OrderStatus value;"
                                                                    "1,NEW,New order;"
                                                                    "2,PARTIALLYFILL,Partially fill;"
                                                                    "3,FILL,Fill;"
                                                                    "4,CANCELLED,Cancelled;"
                                                                    "5,REPLACED,Replaced;"
                                                                    "6,PENDINGCANCEL,Pending cancel;"
                                                                    "7,REJECTED,Rejected;"
                                                                    "8,PENDINGREPLACE,Pending replace;"
                                                                    "9,PENDINGNEW,Pending new"
                                                                  );

    qDebug().nospace().noquote() << "Fill 'ORDER_TYPE' table: " 
                                 << pDbMan->insertToBulkFromString( "ORDER_TYPE"
                                                                  , "0,INVALID,Invalid OrderType value;"
                                                                    "1,LIMIT,Limit;"
                                                                    "2,MARKET,Market"
                                                                  );





    //cout<<"Found config file: " << lookupForConfigFile( "config.properties", "conf;config", FileReadable() ).toStdString() << endl;
    //cout<<"Found config file: " << lookupForConfigFile( "auth.properties", "conf;config"  , FileReadable() ).toStdString() << endl;
    
    return 0;
}



