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


    cout<<"Path to exe   : "<<QCoreApplication::applicationDirPath().toStdString()<<endl;

    cout << endl;

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

    //pDbMan->setDefDecimal(18,8);
    pDbMan->applyDefDecimalFromConfig( *pDatabaseConfig );

    int curLevel = 0;

    QSet<QString> tablesForCreation = pDbMan->tableGetTableNamesForCreation( curLevel );
    for(; !tablesForCreation.empty(); ++curLevel, tablesForCreation = pDbMan->tableGetTableNamesForCreation( curLevel ) )
    {
        cout<<endl<<"Level "<<curLevel<<endl<<endl;

        for( auto tableName : tablesForCreation )
        {
            //cout<<"    "<<tableName.toStdString()<<endl;

            auto expandStrForTableName = cpp::makeExpandString( tableName.toStdString(), 20 );
            auto qexp = QString::fromStdString(expandStrForTableName);

            qDebug().nospace().noquote() << "Drop   table '" << tableName << "'" << qexp << ": " << pDbMan->tableDrop(tableName);
            qDebug().nospace().noquote() << "Create table '" << tableName << "'" << qexp << ": " << pDbMan->tableCreate(tableName);
            qDebug().nospace().noquote() <<"\n";

        }
    }



    //cout<<"Found config file: " << lookupForConfigFile( "config.properties", "conf;config", FileReadable() ).toStdString() << endl;
    //cout<<"Found config file: " << lookupForConfigFile( "auth.properties", "conf;config"  , FileReadable() ).toStdString() << endl;
    
    return 0;
}



