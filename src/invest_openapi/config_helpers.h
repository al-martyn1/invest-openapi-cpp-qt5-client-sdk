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
    bool operator()( const QString &fileFullName )
    {
        //QFile qf = QFile(fileFullName);
        QFile qf{fileFullName};
        return qf.exists();
    }
};

//------------------------------
struct FileReadable
{
    bool operator()( const QString &fileFullName )
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
    bool operator()( const QString &fileFullName )
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
                                     , bool                    useSubfoldersFirst = true
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
QStringList lookupForConfigFiles( const QString                              &ConfigFileName
                                , const QStringList                          &configSubfolderNames  = QStringList()
                                , const LookupCriteriaFileAvailabilityType   &availabilityType      = FileReadable()
                                , const QString                              &startPath             = QCoreApplication::applicationDirPath() // applicationExeDir
                                )
{

    return QString();
}
//QDir::absolutePath() 
//[static]QString QCoreApplication::applicationFilePath()
// bool QFileInfo::isReadable() 
// bool QFileInfo::isRoot()
// bool QDir::cdUp()
// bool QDir::exists(const QString &name) // for file
// bool QDir::exists() // for dir

// QString QDir::dirName() const return empty for root dir
// bool QFile::exists()


} // namespace config_helpers

} // namespace invest_openapi

