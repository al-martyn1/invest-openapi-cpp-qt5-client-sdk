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
#include "invest_openapi/qt_time_helpers.h"

#include "invest_openapi/db_utils.h"

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

    //qDebug().nospace().noquote() << "Main DB name: " << pDatabaseConfig->dbMainFilename;

    ////
    /*
    QSharedPointer<QSqlDatabase> pMainSqlDb = QSharedPointer<QSqlDatabase>( new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE")) );
    pMainSqlDb->setDatabaseName( pDatabaseConfig->dbMainFilename );

    if (!pMainSqlDb->open())
    {
      qDebug() << pMainSqlDb->lastError().text();
      return 1;
    }

    QSharedPointer<tkf::IDatabaseManager> pMainDbMan = tkf::createMainDatabaseManager( pMainSqlDb, pDatabaseConfig, pLoggingConfig );

    //pMainDbMan->setDefaultDecimalFormat(18,8);
    */
    ////

    bool createMainDb    = ( pDatabaseConfig->createSet.find("MAIN")!=pDatabaseConfig->createSet.end() );
    bool createCandlesDb = ( pDatabaseConfig->createSet.find("CANDLES")!=pDatabaseConfig->createSet.end() );
    bool createUserDb    = ( pDatabaseConfig->createSet.find("USER")!=pDatabaseConfig->createSet.end() );


    if (pDatabaseConfig->dbMainFilename.isEmpty() || !createMainDb)
    {
        qDebug().nospace().noquote() << "Creating Main DB: skipped";
        if (!createMainDb)
            qDebug().nospace().noquote() << "Skipped due creation list 'database.create'";
        else
            qDebug().nospace().noquote() << "Skipped due DB name not taken";
    }
    else
    {
        INVEST_OPENAPI_OPEN_DATABASE( pMainSqlDb   , pMainDbMan   , createMainDatabaseManager   , pLoggingConfig, pDatabaseConfig, dbMainFilename    );
        qDebug().nospace().noquote() << "Creating Main DB: " << pDatabaseConfig->dbMainFilename;
       
        int 
        curLevel = 0;
       
        QSet<QString> 
        tablesForCreation = pMainDbMan->tableGetTableNamesForCreation( curLevel );
        for(; !tablesForCreation.empty(); ++curLevel, tablesForCreation = pMainDbMan->tableGetTableNamesForCreation( curLevel ) )
        {
            qDebug().nospace().noquote() << "Level "<<curLevel;
            
            for( auto tableName : tablesForCreation )
            {
                if (pDatabaseConfig->mainInitDropTables)
                    pMainDbMan->tableDrop( tableName, tkf::IDatabaseManager::IfExists::ifExists );
       
                bool createRes = pMainDbMan->tableCreate(tableName, tkf::IDatabaseManager::IfExists::ifNotExists );
                qDebug().nospace().noquote() 
                        << "Create table '" << tableName << "'" 
                        << QString::fromStdString(cpp::makeExpandString( tableName.toStdString(), 20 )) << ": " 
                        << createRes; // <<", time elapsed: " << elapsed;
            }
        }
       
        qDebug().nospace().noquote() << "\n";
       
        if (!pDatabaseConfig->mainInitTablesList.empty())
            qDebug().nospace().noquote() << "Filling Main DB with predefined data";
       
        pMainDbMan->initTablesWithPredefinedData( pDatabaseConfig->mainInitTablesList
                                                , pDatabaseConfig->initFailOnFillUnknownTable
                                                , pDatabaseConfig->initFailOnInsertionError
                                                );
       
        qDebug().nospace().noquote() << "\n";
    }


    //----------------------------------------------------------------------------

    if (pDatabaseConfig->dbCandlesFilename.isEmpty() || !createCandlesDb)
    {
        qDebug().nospace().noquote() << "Creating Candles DB: skipped";
        if (!createCandlesDb)
            qDebug().nospace().noquote() << "Skipped due creation list 'database.create'";
        else
            qDebug().nospace().noquote() << "Skipped due DB name not taken";

    }
    else
    {
        INVEST_OPENAPI_OPEN_DATABASE( pCandlesSqlDb, pCandlesDbMan, createCandlesDatabaseManager, pLoggingConfig, pDatabaseConfig, dbCandlesFilename );
        qDebug().nospace().noquote() << "Creating Candles DB: " << pDatabaseConfig->dbCandlesFilename;
       
        curLevel = 0;
       
        tablesForCreation = pCandlesDbMan->tableGetTableNamesForCreation( curLevel );
        for(; !tablesForCreation.empty(); ++curLevel, tablesForCreation = pCandlesDbMan->tableGetTableNamesForCreation( curLevel ) )
        {
            qDebug().nospace().noquote() << "Level "<<curLevel;
            
            for( auto tableName : tablesForCreation )
            {
                if (pDatabaseConfig->candlesInitDropTables)
                    pCandlesDbMan->tableDrop( tableName, tkf::IDatabaseManager::IfExists::ifExists );
       
                bool createRes = pCandlesDbMan->tableCreate(tableName, tkf::IDatabaseManager::IfExists::ifNotExists );
                qDebug().nospace().noquote() 
                        << "Create table '" << tableName << "'" 
                        << QString::fromStdString(cpp::makeExpandString( tableName.toStdString(), 20 )) << ": " 
                        << createRes; // <<", time elapsed: " << elapsed;
            }
        }
       
        qDebug().nospace().noquote() << "\n";
    }


    //----------------------------------------------------------------------------

    if (pDatabaseConfig->dbUserFilename.isEmpty() || !createUserDb)
    {
        qDebug().nospace().noquote() << "Creating User DB: skipped"; // ", DB name not taken";
        if (!createUserDb)
            qDebug().nospace().noquote() << "Skipped due creation list 'database.create'";
        else
            qDebug().nospace().noquote() << "Skipped due DB name not taken";
    }
    else
    {
        INVEST_OPENAPI_OPEN_DATABASE( pUserSqlDb   , pUserDbMan   , createUserDatabaseManager   , pLoggingConfig, pDatabaseConfig, dbUserFilename    );
        qDebug().nospace().noquote() << "Creating User DB: " << pDatabaseConfig->dbUserFilename;
       
        curLevel = 0;
       
        tablesForCreation = pUserDbMan->tableGetTableNamesForCreation( curLevel );
        for(; !tablesForCreation.empty(); ++curLevel, tablesForCreation = pUserDbMan->tableGetTableNamesForCreation( curLevel ) )
        {
            qDebug().nospace().noquote() << "Level "<<curLevel;
            
            for( auto tableName : tablesForCreation )
            {
                if (pDatabaseConfig->candlesInitDropTables)
                    pUserDbMan->tableDrop( tableName, tkf::IDatabaseManager::IfExists::ifExists );
       
                bool createRes = pUserDbMan->tableCreate(tableName, tkf::IDatabaseManager::IfExists::ifNotExists );
                qDebug().nospace().noquote() 
                        << "Create table '" << tableName << "'" 
                        << QString::fromStdString(cpp::makeExpandString( tableName.toStdString(), 20 )) << ": " 
                        << createRes; // <<", time elapsed: " << elapsed;
            }
        }
       
        qDebug().nospace().noquote() << "\n";
    }


    //----------------------------------------------------------------------------
    qDebug().nospace().noquote() << "Creating Databases - Done";

    
    return 0;
}



