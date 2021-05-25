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

        if (dbFilename.isEmpty())
            return dbFilename;

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

QSharedPointer<IDatabaseManager> 
createCandlesDatabaseManager( QSharedPointer<QSqlDatabase> pDb
                     , QSharedPointer<DatabaseConfig> pDatabaseConfig
                     , QSharedPointer<LoggingConfig> pLoggingConfig 
                     );

QSharedPointer<IDatabaseManager> 
createUserDatabaseManager( QSharedPointer<QSqlDatabase> pDb
                     , QSharedPointer<DatabaseConfig> pDatabaseConfig
                     , QSharedPointer<LoggingConfig> pLoggingConfig 
                     );



struct DatabaseConfig
{
    QString   dbMainFilename;
    QString   dbCandlesFilename;
    QString   dbUserFilename;
    bool      reopenMode; // reopen for each query or not

    std::set<QString> createSet;

    enum CreationType
    {
        full_clean,
        fill_predefs,
        full_fill
    };


    unsigned  defaultDecimalFormatTotalSize      = 18;
    unsigned  defaultDecimalFormatFractionalSize =  8;

    //bool      createClean = false;
    QStringList mainInitTablesList;
    QStringList candlesInitTablesList;
    QStringList userInitTablesList;

    bool mainInitDropTables   ;
    bool candlesInitDropTables;
    bool userInitDropTables   ;


    bool        initFailOnFillUnknownTable  = true;
    bool        initFailOnInsertionError    = true;

    //QString     tableNameInstruments;


    friend QSharedPointer<IDatabaseManager> createMainDatabaseManager( QSharedPointer<QSqlDatabase> pDb, QSharedPointer<DatabaseConfig> pDatabaseConfig, QSharedPointer<LoggingConfig> pLoggingConfig );
    friend QSharedPointer<IDatabaseManager> createCandlesDatabaseManager( QSharedPointer<QSqlDatabase> pDb, QSharedPointer<DatabaseConfig> pDatabaseConfig, QSharedPointer<LoggingConfig> pLoggingConfig );
    friend QSharedPointer<IDatabaseManager> createUserDatabaseManager( QSharedPointer<QSqlDatabase> pDb, QSharedPointer<DatabaseConfig> pDatabaseConfig, QSharedPointer<LoggingConfig> pLoggingConfig );


    void load( const QSettings &settings )
    {
        if (dbMainFilename.isEmpty())
        {
            dbMainFilename = settings.value("database.main").toString();
        }

        if (dbCandlesFilename.isEmpty())
        {
            dbCandlesFilename = settings.value("database.candles").toString();
        }

        if (dbUserFilename.isEmpty())
        {
            dbUserFilename = settings.value("database.user").toString();
        }


        {
            QStringList createList = settings.value("database.create").toStringList();
            for( auto s : createList )
            {
                createSet.insert(s.toUpper());
            }
        }

        reopenMode  = settings.value("database.reopen", QVariant(false)).toBool();
        //createClean = settings.value("database.create_clean", QVariant(false)).toBool();

        mainInitTablesList          = settings.value("database.main.init.tables_list").toStringList();
        candlesInitTablesList       = settings.value("database.candles.init.tables_list").toStringList();
        userInitTablesList          = settings.value("database.user.init.tables_list").toStringList();

        mainInitDropTables          = settings.value("database.main.init.drop_tables", QVariant(false)).toBool();
        candlesInitDropTables       = settings.value("database.candles.init.drop_tables", QVariant(false)).toBool();
        userInitDropTables          = settings.value("database.user.init.drop_tables", QVariant(false)).toBool();

        initFailOnFillUnknownTable  = settings.value("database.init.fail.on_fill_unknown_table"  , QVariant(true)).toBool();
        initFailOnInsertionError    = settings.value("database.init.fail.on_insertion_error"     , QVariant(true)).toBool();


        //tableNameInstruments = settings.value("database.schema.table.name.instruments").toString();

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
        // dbFilename = settings.value("database").toString();
        // dbFilename = dbPlacementStrategy(dbConfigFullName, dbFilename, confPathNamesList);

        load(settings);

        dbMainFilename    = dbPlacementStrategy(dbConfigFullName, dbMainFilename   , confPathNamesList);
        dbCandlesFilename = dbPlacementStrategy(dbConfigFullName, dbCandlesFilename, confPathNamesList);
        dbUserFilename    = dbPlacementStrategy(dbConfigFullName, dbUserFilename   , confPathNamesList);

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

    /*
    DatabaseConfig escapeForDb( const QSqlDatabase &db ) const
    {
        DatabaseConfig res;
        res.dbFilename = dbFilename;
        res.tableNameInstruments = sqlEscape(db, tableNameInstruments);
        return res;
    }
    */

};




} // namespace invest_openapi

