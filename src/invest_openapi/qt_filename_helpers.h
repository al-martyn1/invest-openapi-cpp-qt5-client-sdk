#pragma once


#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>


//----------------------------------------------------------------------------
namespace qt_helpers
{



//----------------------------------------------------------------------------
inline
QString makeCanonicalDirName( QString p )
{
    QDir dir = QDir(p);

    QString resName = dir.canonicalPath();
    if (resName.isEmpty())
        resName = dir.absolutePath();

    return resName;
}

//----------------------------------------------------------------------------
inline
QString makeCanonicalFileName( QString p )
{
    // Not work on non-existent files
    QFileInfo qFileInfo = p;

    QString resName = qFileInfo.canonicalFilePath();
    if (resName.isEmpty())
        resName = qFileInfo.absoluteFilePath();
    return resName;

    //return makeCanonicalDirName(p);
}

//----------------------------------------------------------------------------
inline
QDir getParentDirFromPath( QString fullName, QString *pName = 0)
{
    QFileInfo qFileInfo = QFileInfo( makeCanonicalFileName(fullName) );

    if (pName)
       *pName = qFileInfo.baseName();

    return qFileInfo.absoluteDir();
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
} // namespace qt_helpers


