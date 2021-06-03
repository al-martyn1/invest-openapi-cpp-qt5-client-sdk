/*! \file
    \brief 
 */

//----------------------------------------------------------------------------
#pragma once

#include <QString>
#include <QSettings>
#include <QCoreApplication>

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

    QString  dataLogBaseFolder;

    //----------------------------------------------------------------------------



    //----------------------------------------------------------------------------
    void load( const QSettings &settings )
    {
        debugRequests   = settings.value("logging.debug.dump.requests" , QVariant(false)).toBool();
        debugResponses  = settings.value("logging.debug.dump.responses", QVariant(false)).toBool();
        debugSqlQueries = settings.value("logging.debug.dump.sql"      , QVariant(false)).toBool();

        dataLogBaseFolder = settings.value("logging.data.base-folder").toString();

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


