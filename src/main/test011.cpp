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

#include "invest_openapi/database_config.h"
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

    cout<<"Launched exe  : "<<QCoreApplication::applicationFilePath().toStdString()<<endl;

    namespace tkf=invest_openapi;
    using tkf::config_helpers::lookupForConfigFile;
    using tkf::config_helpers::FileReadable;

    QString lookupConfSubfolders = "conf;config";

    auto dbConfigFullName  = lookupForConfigFile( "database.properties", lookupConfSubfolders, FileReadable() );
    auto logConfigFullName = lookupForConfigFile( "logging.properties" , lookupConfSubfolders, FileReadable() );

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

    pDbMan->setDefaultDecimalFormat(18,8);

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


    return 0;
}



