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
    QCoreApplication::setApplicationName("test012");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;

    cout<<"Launched exe : "<<QCoreApplication::applicationFilePath().toStdString()<<endl;

    namespace tkf=invest_openapi;
    using tkf::config_helpers::lookupForConfigFile;
    using tkf::config_helpers::FileReadable;

    QString lookupConfSubfolders = "conf;config";

    auto dbConfigFullName  = lookupForConfigFile( "database.properties", lookupConfSubfolders, FileReadable() );
    auto logConfigFullName = lookupForConfigFile( "logging.properties" , lookupConfSubfolders, FileReadable() );

    QSharedPointer<tkf::DatabaseConfig> pDatabaseConfig = QSharedPointer<tkf::DatabaseConfig>( new tkf::DatabaseConfig(dbConfigFullName, tkf::DatabasePlacementStrategyDefault()) );
    QSharedPointer<tkf::LoggingConfig>  pLoggingConfig  = QSharedPointer<tkf::LoggingConfig> ( new tkf::LoggingConfig(logConfigFullName) );


    qDebug().nospace().noquote() << "DB name      : " << pDatabaseConfig->dbFilename;

    qDebug() << "";

    QSharedPointer<QSqlDatabase> pSqlDb = QSharedPointer<QSqlDatabase>( new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE")) );
    pSqlDb->setDatabaseName( pDatabaseConfig->dbFilename );

    if (!pSqlDb->open())
    {
      qDebug() << pSqlDb->lastError().text();
      return 0;
    }


    QSharedPointer<tkf::IDatabaseManager> pDbMan = tkf::createDatabaseManager( pSqlDb, pDatabaseConfig, pLoggingConfig );

    pDbMan->setDefDecimal(18,8);

    qDebug().nospace().noquote() << pDbMan->getTableExistString("INSTRUMENTS");
    qDebug().nospace().noquote() << pDbMan->getTableExistString("CURRENCIES");


    qDebug() << "";

    qDebug().nospace().noquote() << "Drop   table CURRENCIES        : " << pDbMan->tableDrop("CURRENCIES");
    qDebug().nospace().noquote() << "Create table CURRENCIES        : " << pDbMan->tableCreate("CURRENCIES");

    qDebug() << "";

    qDebug().nospace().noquote() << "Drop   table INSTRUMENT_TYPES  : " << pDbMan->tableDrop("INSTRUMENT_TYPES");
    qDebug().nospace().noquote() << "Create table INSTRUMENT_TYPES  : " << pDbMan->tableCreate("INSTRUMENT_TYPES");

    qDebug() << "";

    qDebug().nospace().noquote() << "Drop   table INSTRUMENTS  : " << pDbMan->tableDrop("INSTRUMENTS");
    qDebug().nospace().noquote() << "Create table INSTRUMENTS  : " << pDbMan->tableCreate("INSTRUMENTS");



    tkf::IOaDatabaseManager *pOaDbMan = dynamic_cast<tkf::IOaDatabaseManager*>(pDbMan.get());
    if (!pOaDbMan)
        throw std::runtime_error("dynamic_cast<tkf::IOaDatabaseManager*>() failed");

    qDebug() << "";

    pOaDbMan->insertNewCurrencyTypes   ("UNK:Unknown;RUB:Russian ruble;USD:US Dollar;EUR:European Euro;GBP:Great Britain Pound;HKD:Hong Kong Dollar;CHF:Swiss Franc;JPY:Japanese Yen;CNY:Chinese Yuan;TRY:Turkish Lira");
    pOaDbMan->insertNewInstrumentTypes ("UNK:Unknown;Stock:Stock;Bond:Bond;Currency:Currency;Etf:Etf");
    


    return 0;
}



