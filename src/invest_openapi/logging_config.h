/*! \file
    \brief 
 */

//----------------------------------------------------------------------------
#pragma once

#include <QString>
#include <QSettings>
#include <QCoreApplication>
#include <QStandardPaths>

#include <exception>
#include <stdexcept>

#include "qt_filename_helpers.h"

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
namespace invest_openapi
{



//----------------------------------------------------------------------------
struct LoggingConfig
{
    bool debugRequests ;
    bool debugResponses;
    bool debugSqlQueries;

    QString      dataLogBaseFolder;
    QStringList  logHandlers;        // cout/cerr/file


    QString      logFilenameDateTimeFormat;
    QString      logFilenameDateFormat;
    QString      logFilenameTimeFormat;

    QString      logDateTimeFormat;
    int          logLevel          = 7;
    bool         logContext        = true;

    QString      logFileName;


    //----------------------------------------------------------------------------
    const std::map<QString, int>& getLogLevelsMap() const
    {
        static std::map<QString, int> llMap;

        if (llMap.empty())
        {
            llMap[ QString("EMERG"     ) ] = 0;
            llMap[ QString("EMERGENCY" ) ] = 0;
            llMap[ QString("0"         ) ] = 0;
            llMap[ QString("ALERT"     ) ] = 1;
            llMap[ QString("1"         ) ] = 1;
            llMap[ QString("CRIT"      ) ] = 2;
            llMap[ QString("CRITICAL"  ) ] = 2;
            llMap[ QString("2"         ) ] = 2;
            llMap[ QString("ERROR"     ) ] = 3;
            llMap[ QString("3"         ) ] = 3;
            llMap[ QString("WARN"      ) ] = 4;
            llMap[ QString("WARNING"   ) ] = 4;
            llMap[ QString("4"         ) ] = 4;
            llMap[ QString("NOTICE"    ) ] = 5;
            llMap[ QString("5"         ) ] = 5;
            llMap[ QString("INFO"      ) ] = 6;
            llMap[ QString("6"         ) ] = 6;
            llMap[ QString("DEBUG"     ) ] = 7;
            llMap[ QString("7"         ) ] = 7;
        }

        return llMap;
        
    }

    //----------------------------------------------------------------------------
    int logLevelFromString( QString s )
    {
        const std::map<QString, int> &llMap = getLogLevelsMap();

        std::map<QString, int>::const_iterator lit = llMap.find(s.toUpper());

        if (lit==llMap.end())
           throw std::runtime_error( std::string("invest_openapi::LoggingConfig::load: unknown log level: '") + s.toStdString() + std::string("' in 'logging.log.level' property") );

        return lit->second;
    }

    //----------------------------------------------------------------------------
    void load( const QSettings &settings )
    {
        debugRequests     = settings.value("logging.debug.dump.requests" , QVariant(false)).toBool();
        debugResponses    = settings.value("logging.debug.dump.responses", QVariant(false)).toBool();
        debugSqlQueries   = settings.value("logging.debug.dump.sql"      , QVariant(false)).toBool();

        dataLogBaseFolder = settings.value("logging.data.base-folder").toString();

        logHandlers       = settings.value("logging.log.handlers").toStringList();

        for( auto &h : logHandlers )
        {
            auto tmp = h;

            h = h.toLower();
            if (h=="cout" || h=="cerr" || h=="file")
                continue;

            throw std::runtime_error( std::string("invest_openapi::LoggingConfig::load: unknown log handler: '") + tmp.toStdString() + std::string("' in 'logging.log.handlers' property") );
        }


        logFilenameDateTimeFormat = settings.value("logging.log.file.name.datetime.format", "yyyy_MM_dd-hh_mm_ss").toString();
        logFilenameDateFormat     = settings.value("logging.log.file.name.date.format"    , "yyyy_MM_dd").toString();
        logFilenameTimeFormat     = settings.value("logging.log.file.name.time.format"    , "hh_mm_ss").toString();

        logDateTimeFormat         = settings.value("logging.log.datetime.format"          , "yyyy-MM-dd hh:mm:ss.zzz").toString();

        //[2020-12-23 01:52:04.069]
        // yyyy-MM-dd hh:mm:ss.zzz

        logFileName = settings.value("logging.log.file.name" ).toString();

        logLevel    = logLevelFromString( settings.value("logging.log.level", "DEBUG" ).toString() );

        logContext  = settings.value("logging.log.context", false ).toBool();

    }


    //------------------------------
    QString getLogFileName( QString configFullName ) const
    {
        QFileInfo qFileInfo      = QFileInfo(configFullName);
        QString   confPath       = qFileInfo.absolutePath();
        QString   confFilePath   = qFileInfo.absoluteFilePath();

        //QDir      confDir     = qFileInfo.absolutePath();
        //QString   confDirName = confDir.dirName();


        QString   logFileNameProcessed = logFileName;

        QDateTime dtNow = QDateTime::currentDateTime();

        std::map< QString, QString > vars;

        vars["$(AppName)" ] = QCoreApplication::applicationName();
        vars["$(HomeDir)" ] = QDir::homePath(); // .absolutePath();
        vars["$(ConfDir)" ] = confPath; // confDir.dirName();
        vars["$(AppBin)"  ] = QCoreApplication::applicationDirPath();

        vars["$(DateTime)"] = dtNow.toString(logFilenameDateTimeFormat);
        vars["$(Date)"    ] = dtNow.toString(logFilenameDateFormat    );
        vars["$(Time)"    ] = dtNow.toString(logFilenameTimeFormat    );


        std::map< QString, QString >::const_iterator vit = vars.begin();
        for(; vit!=vars.end(); ++vit)
        {
            int varPos = logFileNameProcessed.indexOf( vit->first, 0, Qt::CaseInsensitive );
            while(varPos>=0)
            {
                logFileNameProcessed.replace( varPos, vit->first.size(), vit->second );
                varPos = logFileNameProcessed.indexOf( vit->first, varPos + vit->second.size(), Qt::CaseInsensitive );
            }
        }


        QFileInfo qFileInfoLogFile = QFileInfo(logFileNameProcessed);
        QDir      logFileDir       = qFileInfoLogFile.absoluteDir();

        QString   logFileDirOnlyName;
        //QString   logFileDirName = logFileDir.dirName();
        QString   logFileDirName = logFileDir.absolutePath();
        QDir      logDirParent = qt_helpers::getParentDirFromPath( logFileDirName, &logFileDirOnlyName );
        bool mkdirRes    = logDirParent.mkpath(logFileDirOnlyName);

        return logFileNameProcessed;
    
    }

    //------------------------------
    std::map<QString, QString> getLogHandlers( QString configFullName ) const
    {
        std::map<QString, QString> handlersMap;

        for( const auto &h : logHandlers )
        {
            if (h=="cout" || h=="cerr")
                handlersMap[h] = QString();
            else if (h=="file")
                handlersMap[h] = getLogFileName(configFullName);
            else
                throw std::runtime_error( std::string("invest_openapi::LoggingConfig::load: unknown log handler: '") + h.toStdString() + std::string("' in 'logHandlers' member") );
        }

        return handlersMap;
    
    }

    //------------------------------
    QString getDataLogBaseFolder( QString configFullName ) const
    {
        if (dataLogBaseFolder.isEmpty())
            return dataLogBaseFolder;

        QFileInfo qFileInfo = QFileInfo(configFullName);
        QDir      confDir   = qFileInfo.absoluteDir();

        QString   dataLogBaseFolderFullPath          = confDir.absoluteFilePath(dataLogBaseFolder);
        QString   dataLogBaseFolderFullPathCanonical = qt_helpers::makeCanonicalFileName(dataLogBaseFolderFullPath);


        QString   dataLogBaseFolderName;
        QDir      dataLogDirParent = qt_helpers::getParentDirFromPath(dataLogBaseFolderFullPathCanonical, &dataLogBaseFolderName );
        bool mkdirRes = dataLogDirParent.mkpath(dataLogBaseFolderName);

        return dataLogBaseFolderFullPathCanonical;
    }

    //------------------------------
    QString getDataLogFolder( QString configFullName, QString subFolder ) const
    {
        QString dataFolderName = getDataLogBaseFolder(configFullName);

        if (subFolder.isEmpty())
            subFolder = QCoreApplication::applicationName();

        QDir      dataLogDir       = dataFolderName;
        QString   absPath          = dataLogDir.absoluteFilePath(subFolder);
        QString   absPathCanonical = qt_helpers::makeCanonicalFileName(absPath);

        bool mkdirRes = dataLogDir.mkpath(subFolder); // !!! Почему-то не создаётся, хотя возвращает тру

        return absPathCanonical;
    }

    //------------------------------
    QString getDataLogFullName( QString configFullName, QString subFolder, QString fileName ) const
    {
        if (fileName.isEmpty())
            throw std::runtime_error("invest_openapi::LoggingConfig::getDataLogFullName - missing file name");

        QString folderFullName = getDataLogFolder( configFullName, subFolder );
        QDir    dataLogDir     = folderFullName;
        QFileInfo qFileInfo    = QFileInfo(dataLogDir, fileName);

        QString resName = qFileInfo.absoluteFilePath();
        QString resNameCanonical = qt_helpers::makeCanonicalFileName(resName);
        return resNameCanonical;
    }

    //------------------------------

    void checkValid() const
    {
    }

    LoggingConfig( const QSettings &settings )
    {
        load(settings);
        checkValid();
    }

    LoggingConfig( const QString &settingsFile )
    {
        QSettings settings(settingsFile, QSettings::IniFormat);
        load(settings);
        checkValid();
    }

};

//----------------------------------------------------------------------------



} // namespace invest_openapi


