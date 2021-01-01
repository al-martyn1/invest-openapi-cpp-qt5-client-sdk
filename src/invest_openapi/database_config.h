#pragma once

#include <QObject>
#include <QVariant>
#include <QString>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlField>
#include <QSqlQuery>




namespace invest_openapi
{


struct DatabasePlacementStrategyDefault
{
    // use pathListSplit if needed to split single QString pathList  to QStringList

    QString operator()( const QString &dbConfigFullName, const QString &dbFilename, const QStringList &confPathNamesList = QStringList() ) const
    {
        // confPathNamesList ignored in default strategy
        // 

        QFileInfo qFileInfo = QFileInfo(dbConfigFullName);
        QDir      confDir   = qFileInfo.absoluteDir();
        return    confDir   . absoluteFilePath(dbFilename);
    }

}; // struct DatabasePlacementStrategyDefault




class DatabaseConfig;
class IDatabaseManager;
QSharedPointer<IDatabaseManager> createDatabaseManager( QSharedPointer<QSqlDatabase> pDb, QSharedPointer<DatabaseConfig> pDatabaseConfig, QSharedPointer<LoggingConfig> pLoggingConfig );




struct DatabaseConfig
{
    QString   dbFilename;
    bool      reopenMode; // reopen for each query or not

    QString   tableNameInstruments;


    friend QSharedPointer<IDatabaseManager> createDatabaseManager( QSharedPointer<QSqlDatabase> pDb, QSharedPointer<DatabaseConfig> pDatabaseConfig, QSharedPointer<LoggingConfig> pLoggingConfig );



    void load( const QSettings &settings )
    {
        if (dbFilename.isEmpty())
        {
            dbFilename = settings.value("database").toString();
        }

        reopenMode = settings.value("database.reopen", QVariant(false)).toBool();

        tableNameInstruments = settings.value("database.schema.table.name.instruments").toString();
    }

    template<typename DatabasePlacementStrategyType>
    void load( const QSettings &settings, const QString &dbConfigFullName, const DatabasePlacementStrategyType &dbPlacementStrategy, const QStringList &confPathNamesList = QStringList() )
    {
        dbFilename = settings.value("database").toString();
        dbFilename = dbPlacementStrategy(dbConfigFullName, dbFilename, confPathNamesList);

        load(settings);
    }

    void checkValid()
    {}

    DatabaseConfig( ) {}

    DatabaseConfig( const QSettings &settings )
    {
        load(settings);
        checkValid();
    }

    template<typename DatabasePlacementStrategyType>
    DatabaseConfig( const QSettings &settings, const QString &dbConfigFullName, const DatabasePlacementStrategyType &dbPlacementStrategy, const QStringList &confPathNamesList = QStringList() )
    {
        load( settings, dbConfigFullName, dbPlacementStrategy, confPathNamesList );
        checkValid();
    }

    template<typename DatabasePlacementStrategyType>
    DatabaseConfig( const QString &dbConfigFullName, const DatabasePlacementStrategyType &dbPlacementStrategy, const QStringList &confPathNamesList = QStringList() )
    {
        QSettings settings(dbConfigFullName, QSettings::IniFormat);
        load( settings, dbConfigFullName, dbPlacementStrategy, confPathNamesList );
        checkValid();
    }


protected:

    DatabaseConfig escapeForDb( const QSqlDatabase &db ) const
    {
        DatabaseConfig res;
        res.dbFilename = dbFilename;
        res.tableNameInstruments = sqlEscape(db, tableNameInstruments);
        return res;
    }

};




} // namespace invest_openapi

