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
    cout<<"Found config file: " << dbConfigFullName.toStdString() << endl;

    tkf::DatabasePlacementStrategyDefault defStrategy = tkf::DatabasePlacementStrategyDefault();
    auto dbName = defStrategy( dbConfigFullName, "dbPathTest.sql" );
    cout<<"DB name: " << dbName.toStdString() << endl;

    tkf::DatabaseConfig databaseConfig = tkf::DatabaseConfig(dbConfigFullName, tkf::DatabasePlacementStrategyDefault() );


    //QSqlDatabase sqlDb = QSqlDatabase::addDatabase("QSQLITE");
    //sqlDb.setDatabaseName(databaseConfig.dbFilename);
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

    QSharedPointer<tkf::IDatabaseManager> pDbMan = tkf::createDatabaseManager( pSqlDb, databaseConfig );

    qDebug() << "Table INSTRUMENTS: " << (pDbMan->tableCheckExist( "INSTRUMENTS" ) ? "EXIST" : "NOT EXIST");



    return 0;
}



