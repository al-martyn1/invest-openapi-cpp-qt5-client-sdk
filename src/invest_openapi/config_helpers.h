/*! \file
    \brief 
 */

#include <QVector>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QCoreApplication>
#include <QFile>

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
namespace invest_openapi
{

namespace config_helpers
{

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
struct FileExists
{
    bool operator()( const QString &fileFullName ) const
    {
        //QFile qf = QFile(fileFullName);
        QFile qf{fileFullName};
        return qf.exists();
    }
};

//------------------------------
struct FileReadable
{
    bool operator()( const QString &fileFullName ) const
    {
        //QFile qf = QFile(fileFullName);
        QFile qf{fileFullName};
        if (!qf.open(QIODevice::ReadOnly))
            return false;
        qf.close();
        return true;
    }
};

//------------------------------
struct FileWriteable
{
    bool operator()( const QString &fileFullName ) const
    {
        //QFile qf = QFile(fileFullName);
        QFile qf{fileFullName};
        if (!qf.open(QIODevice::Append)) // do not truncate
            return false;
        qf.close();
        return true;
    }
};

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
inline
QStringList generateFileNamesUpToRoot( const QString          &fileName
                                     , const QStringList      &subfolderNames
                                     , const QString          &startPath
                                     , bool                    useSubfoldersFirst  = true
                                     , bool                    skipNonexistentDirs = true
                                     , unsigned                maxUpLevel          = (unsigned)-1
                                     )
{
    QStringList resList;

    QDir dir = QDir(startPath);

    unsigned upLevel = 0;

    while(!dir.isRoot() && upLevel<=maxUpLevel)
    {
        if (!useSubfoldersFirst)
        {
            QString n = dir.filePath(fileName);
            resList.append(n);
        }

        for (int i = 0; i < subfolderNames.size(); ++i)
        {
            QString subfolderName = subfolderNames.at(i).trimmed();
            if (subfolderName.isEmpty())
                continue;

            QDir tmpDir = dir.filePath(subfolderName);
            if (!tmpDir.exists())
            {
                if (skipNonexistentDirs)
                    continue;
            }

            QString n = tmpDir.filePath(fileName);
            resList.append(n);
        }

        if (useSubfoldersFirst)
        {
            QString n = dir.filePath(fileName);
            resList.append(n);
        }

        dir.cdUp();
        upLevel++;
    }

    return resList;
}

//----------------------------------------------------------------------------
inline
QStringList generateFileNamesUpToRoot( const QString          &fileName
                                     , const QString           subfolderNames
                                     , const QString          &startPath
                                     , bool                    useSubfoldersFirst  = true
                                     , bool                    skipNonexistentDirs = true
                                     , unsigned                maxUpLevel          = (unsigned)-1
                                     )
{
    QString subfolderNamesCopy = subfolderNames;
    subfolderNamesCopy.replace(':', ";"); // replace *nix style list separator to windows style separator
    return generateFileNamesUpToRoot( fileName, subfolderNamesCopy.split( ';', Qt::SkipEmptyParts ), startPath, useSubfoldersFirst, skipNonexistentDirs, maxUpLevel );
}

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
template< typename LookupCriteriaFileAvailabilityType >
inline
QStringList lookupForConfigFiles( const QString                              &configFileName
                                , const QStringList                          &configSubfolderNames  = QStringList()
                                , const LookupCriteriaFileAvailabilityType   &availabilityType      = FileReadable()
                                , const QString                              &startPath             = QCoreApplication::applicationDirPath() // applicationExeDir
                                , bool                                        useSubfoldersFirst    = true
                                , unsigned                                    maxUpLevel            = (unsigned)-1
                                )
{
    QStringList resList;

    QStringList foundCandidates = generateFileNamesUpToRoot( configFileName
                                                           , configSubfolderNames
                                                           , startPath
                                                           , useSubfoldersFirst
                                                           , true // skipNonexistentDirs
                                                           , maxUpLevel
                                                           );

    for (int i = 0; i < foundCandidates.size(); ++i)
    {
        QString candidateFileName = foundCandidates.at(i).trimmed();
        
        if (candidateFileName.isEmpty()) 
            continue; // Нахуй пустышки, насрать как появились

        if (!availabilityType(candidateFileName))
            continue; // Неподходящие под критерию - нахуй

        resList.append(candidateFileName);
    }

    return resList;
}

//----------------------------------------------------------------------------
template< typename LookupCriteriaFileAvailabilityType >
inline
QStringList lookupForConfigFiles( const QString                              &configFileName
                                , const QString                              &configSubfolderNames  = QString()
                                , const LookupCriteriaFileAvailabilityType   &availabilityType      = FileReadable()
                                , const QString                              &startPath             = QCoreApplication::applicationDirPath() // applicationExeDir
                                , bool                                        useSubfoldersFirst    = true
                                , unsigned                                    maxUpLevel            = (unsigned)-1
                                )
{
    QString subfolderNamesCopy = configSubfolderNames;
    subfolderNamesCopy.replace(':', ";"); // replace *nix style list separator to windows style separator
    return lookupForConfigFiles( configFileName
                               , subfolderNamesCopy.split( ';', Qt::SkipEmptyParts )
                               , availabilityType
                               , startPath
                               , useSubfoldersFirst
                               , maxUpLevel
                               );
}

//----------------------------------------------------------------------------
template< typename LookupCriteriaFileAvailabilityType >
inline
QString lookupForConfigFile( const QString                              &configFileName
                           , const QStringList                          &configSubfolderNames  = QStringList()
                           , const LookupCriteriaFileAvailabilityType   &availabilityType      = FileReadable()
                           , const QString                              &startPath             = QCoreApplication::applicationDirPath() // applicationExeDir
                           , bool                                        useSubfoldersFirst    = true
                           #if defined(DEBUG) || defined(_DEBUG)
                           , unsigned                                    maxUpLevel            = (unsigned)-1
                           #else
                           , unsigned                                    maxUpLevel            = 1
                           #endif
                           )
{
    QStringList lst = lookupForConfigFiles( configFileName, configSubfolderNames, availabilityType, startPath, useSubfoldersFirst, maxUpLevel );
    if (lst.isEmpty())
        return QString();
    return lst.at(0);
}

//----------------------------------------------------------------------------
template< typename LookupCriteriaFileAvailabilityType >
inline
QString lookupForConfigFile( const QString                              &configFileName
                           , const QString                              &configSubfolderNames  = QString()
                           , const LookupCriteriaFileAvailabilityType   &availabilityType      = FileReadable()
                           , const QString                              &startPath             = QCoreApplication::applicationDirPath() // applicationExeDir
                           , bool                                        useSubfoldersFirst    = true
                           #if defined(DEBUG) || defined(_DEBUG)
                           , unsigned                                    maxUpLevel            = (unsigned)-1
                           #else
                           , unsigned                                    maxUpLevel            = 1
                           #endif
                           )
{
    QString subfolderNamesCopy = configSubfolderNames;
    subfolderNamesCopy.replace(':', ";"); // replace *nix style list separator to windows style separator
    return lookupForConfigFile( configFileName, subfolderNamesCopy.split( ';', Qt::SkipEmptyParts ), availabilityType, startPath, useSubfoldersFirst, maxUpLevel );
}




//----------------------------------------------------------------------------

} // namespace config_helpers

} // namespace invest_openapi

