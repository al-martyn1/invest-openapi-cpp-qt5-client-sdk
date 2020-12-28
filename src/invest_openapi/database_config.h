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


struct DatabaseConfig
{
    QString   dbFilename;
    QString   instrumentsTableName;

    void load( const QSettings &settings )
    {
        if (dbFilename.isEmpty())
        {
            dbFilename = settings.value("database").toString(); // defStrategy( dbConfigFullName, "dbPathTest.sql" );
        }

        instrumentsTableName = settings.value("database.schema.table.name.instruments").toString();
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

    DatabaseConfig escapeForDb( const QSqlDatabase &db ) const
    {
        DatabaseConfig res;
        res.dbFilename = dbFilename;
        res.instrumentsTableName = sqlEscape(db, instrumentsTableName);
        return res;
    }

    bool createDatabaseShema(QSqlDatabase &db) const
    {
        QSqlQuery q(db);
        q.exec(QString("DROP TABLE ") + instrumentsTableName + QString(";") );


        
    }



};




} // namespace invest_openapi

