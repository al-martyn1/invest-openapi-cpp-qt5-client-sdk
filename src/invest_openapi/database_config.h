#pragma once

#include <QObject>
#include <QVariant>
#include <QString>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlField>
#include <QSqlQuery>

#include "logging_config.h"


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
QSharedPointer<IDatabaseManager> 
createMainDatabaseManager( QSharedPointer<QSqlDatabase> pDb
                     , QSharedPointer<DatabaseConfig> pDatabaseConfig
                     , QSharedPointer<LoggingConfig> pLoggingConfig 
                     );




struct DatabaseConfig
{
    QString   dbFilename;
    bool      reopenMode; // reopen for each query or not

    enum CreationType
    {
        full_clean,
        fill_predefs,
        full_fill
    };


    unsigned  defaultDecimalFormatTotalSize      = 18;
    unsigned  defaultDecimalFormatFractionalSize =  8;

    //bool      createClean = false;
    QStringList initTablesList;
    bool        initFailOnFillUnknownTable  = true;
    bool        initFailOnInsertionError    = true;

    QString     tableNameInstruments;


    friend QSharedPointer<IDatabaseManager> createMainDatabaseManager( QSharedPointer<QSqlDatabase> pDb, QSharedPointer<DatabaseConfig> pDatabaseConfig, QSharedPointer<LoggingConfig> pLoggingConfig );



    void load( const QSettings &settings )
    {
        if (dbFilename.isEmpty())
        {
            dbFilename = settings.value("database").toString();
        }

        reopenMode  = settings.value("database.reopen", QVariant(false)).toBool();
        //createClean = settings.value("database.create_clean", QVariant(false)).toBool();

        initTablesList              = settings.value("database.init.tables_list").toStringList();
        initFailOnFillUnknownTable  = settings.value("database.init.fail.on_fill_unknown_table"  , QVariant(true)).toBool();
        initFailOnInsertionError    = settings.value("database.init.fail.on_insertion_error"     , QVariant(true)).toBool();


        tableNameInstruments = settings.value("database.schema.table.name.instruments").toString();

        QString decimalFormatStr = settings.value("database.defaults.decimal.format", QVariant("18.8")).toString();
        // https://doc.qt.io/qt-5/qstring.html#split-5
        QStringList decimalSizeStrList = decimalFormatStr.split( '.', Qt::SkipEmptyParts , Qt::CaseSensitive);
        if (decimalSizeStrList.size()==2)
        {
            defaultDecimalFormatTotalSize       = decimalSizeStrList[0].toUInt();
            defaultDecimalFormatFractionalSize  = decimalSizeStrList[1].toUInt();
        }


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

