/*! \file
    \brief SQLite database test

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
#include <QSqlDatabase>
#include <QSqlDriver>



#include "invest_openapi/config_helpers.h"
#include "invest_openapi/api_config.h"
#include "invest_openapi/auth_config.h"
#include "invest_openapi/currencies_config.h"

#include "invest_openapi/invest_openapi.h"
#include "invest_openapi/factory.h"
#include "invest_openapi/openapi_completable_future.h"

#include "invest_openapi/database_manager.h"


// https://sqlitebrowser.org/
// https://sqlitebrowser.org/blog/version-3-12-1-released/

INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("test011");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;


    cout<<"Launched from : "<<QDir::currentPath().toStdString()<<endl;
    cout<<"Launched exe  : "<<QCoreApplication::applicationFilePath().toStdString()<<endl;
    cout<<"Launched from : "<<QCoreApplication::applicationDirPath().toStdString()<<endl;

    namespace tkf=invest_openapi;
    using tkf::config_helpers::lookupForConfigFile;
    using tkf::config_helpers::FileReadable;

    auto dbConfigFullName = lookupForConfigFile( "database.properties", "conf;config", FileReadable() ); // .toStdString();
    // cout<<"Found config file: " << dbConfigFullName.toStdString() << endl;
    //  
    // tkf::DatabasePlacementStrategyDefault defStrategy = tkf::DatabasePlacementStrategyDefault();
    // auto dbName = defStrategy( dbConfigFullName, "dbPathTest.sql" );
    // cout<<"DB name: " << dbName.toStdString() << endl;

    tkf::DatabaseConfig databaseConfig = tkf::DatabaseConfig(dbConfigFullName, tkf::DatabasePlacementStrategyDefault() );

    //QSqlDatabase sqlDb = QSqlDatabase::addDatabase("QSQLITE");
    //sqlDb.setDatabaseName(databaseConfig.dbFilename);

    qDebug().nospace().noquote() << "DB name: " << databaseConfig.dbFilename;

    QSharedPointer<QSqlDatabase> pSqlDb = QSharedPointer<QSqlDatabase>( new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE")) );
    pSqlDb->setDatabaseName( databaseConfig.dbFilename );

    if (!pSqlDb->open())
    {
      qDebug() << pSqlDb->lastError().text();
      return 0;
    }

    /*
    databaseConfig = databaseConfig.escapeForDb(sqlDb);

    if (!sqlDb.open())
    {
      qDebug() << sqlDb.lastError().text();
      return 0;
    }

    databaseConfig.createDatabaseShema(sqlDb);

    */

    // auto pairs = tkf::simpleSplitToPairs( "USD:US Dollar;EUR,Euro;RUB,Russian ruble" );

    // int         usdInt    = tkf::toInt( tkf::toCurrency("usd") );
    // std::string usdStr    = tkf::toStdString( tkf::toCurrency("usd") );

    QSharedPointer<tkf::IDatabaseManager> pDbMan = tkf::createDatabaseManager( pSqlDb, databaseConfig );

    pDbMan->setDefDecimal(17,11);

    qDebug().nospace().noquote() << pDbMan->getTableExistString("INSTRUMENTS");
    qDebug().nospace().noquote() << pDbMan->getTableExistString("CURRENCIES");

    qDebug() << "";

    qDebug().nospace().noquote() << "Drop table TEST if exists (def)       : " << pDbMan->tableDrop("TEST")                                                << ", SQL Error:" << pSqlDb->lastError();
    qDebug().nospace().noquote() << "Drop table TEST if not exists         : " << pDbMan->tableDrop("TEST", tkf::IDatabaseManager::IfExists::ifNotExists ) << ", SQL Error:" << pSqlDb->lastError();
    qDebug().nospace().noquote() << "Drop table TEST in anyway             : " << pDbMan->tableDrop("TEST", tkf::IDatabaseManager::IfExists::ifAnyway    ) << ", SQL Error:" << pSqlDb->lastError();

    qDebug() << "";

    qDebug().nospace().noquote() << "Create table TEST if not exists (def) : " << pDbMan->tableCreate("TEST")                                              << ", SQL Error:" << pSqlDb->lastError();
    qDebug().nospace().noquote() << "Create table TEST if exists           : " << pDbMan->tableCreate("TEST", tkf::IDatabaseManager::IfExists::ifExists  ) << ", SQL Error:" << pSqlDb->lastError();
    qDebug().nospace().noquote() << "Create table TEST in anyway           : " << pDbMan->tableCreate("TEST", tkf::IDatabaseManager::IfExists::ifAnyway  ) << ", SQL Error:" << pSqlDb->lastError();

    qDebug() << "";
    
    qDebug().nospace().noquote() << "Drop table TEST if not exists         : " << pDbMan->tableDrop("TEST", tkf::IDatabaseManager::IfExists::ifNotExists ) << ", SQL Error:" << pSqlDb->lastError();
    qDebug().nospace().noquote() << "Drop table TEST in anyway             : " << pDbMan->tableDrop("TEST", tkf::IDatabaseManager::IfExists::ifAnyway    ) << ", SQL Error:" << pSqlDb->lastError();

    qDebug() << "";

    qDebug().nospace().noquote() << "Create table TEST if exists           : " << pDbMan->tableCreate("TEST", tkf::IDatabaseManager::IfExists::ifExists  ) << ", SQL Error:" << pSqlDb->lastError();
    qDebug().nospace().noquote() << "Create table TEST in anyway           : " << pDbMan->tableCreate("TEST", tkf::IDatabaseManager::IfExists::ifAnyway  ) << ", SQL Error:" << pSqlDb->lastError();

    qDebug() << "";

    qDebug().nospace().noquote() << "Drop   table CURRENCY_TYPES    : " << pDbMan->tableDrop("CURRENCY_TYPES");
    qDebug().nospace().noquote() << "Create table CURRENCY_TYPES    : " << pDbMan->tableCreate("CURRENCY_TYPES");

    qDebug() << "";

    qDebug().nospace().noquote() << "Drop   table INSTRUMENT_TYPES  : " << pDbMan->tableDrop("INSTRUMENT_TYPES");
    qDebug().nospace().noquote() << "Create table INSTRUMENT_TYPES  : " << pDbMan->tableCreate("INSTRUMENT_TYPES");

    tkf::IOaDatabaseManager *pOaDbMan = dynamic_cast<tkf::IOaDatabaseManager*>(pDbMan.get());
    if (!pOaDbMan)
        throw std::runtime_error("dynamic_cast<tkf::IOaDatabaseManager*>() failed");

    qDebug() << "";

    //qDebug().nospace().noquote() << "Adding USD:" << pOaDbMan->insertNewCurrencyType("USD", "US Dollar");
    pOaDbMan->insertNewCurrencyTypes("RUB:Russian ruble;USD:US Dollar;EUR:European Euro");

    // RUB/USD/EUR/GBP/HKD/CHF/JPY/CNY/TRY




    return 0;
}



