/*! \file
    \brief Configs lookup test
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

#include "invest_openapi/config_helpers.h"
#include "invest_openapi/api_config.h"
#include "invest_openapi/auth_config.h"

#include "invest_openapi/invest_openapi.h"
#include "invest_openapi/factory.h"
#include "invest_openapi/openapi_completable_future.h"



inline
void printQStringList( QString indent, QStringList strings )
{
    using std::cout;
    using std::endl;

    for (int i = 0; i < strings.size(); ++i)
    {
        QString s = strings.at(i).trimmed();
        cout << indent.toStdString() << s.toStdString() << endl;
    }

    cout << endl;

}


INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("test004");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;


    cout<<"Launched from : "<<QDir::currentPath().toStdString()<<endl;
    cout<<"Launched exe  : "<<QCoreApplication::applicationFilePath().toStdString()<<endl;
    cout<<"Launched from : "<<QCoreApplication::applicationDirPath().toStdString()<<endl;


    QStringList 
    configNames = invest_openapi::config_helpers::generateFileNamesUpToRoot( QString("app.config")
                                                                           , QString("conf;config")
                                                                           , QString(QCoreApplication::applicationDirPath())
                                                                           , true  // useSubfoldersFirst
                                                                           , false // skipNonexistentDirs
                                                                           );
    cout<<"Generated names (all, subfolders first, lookup level infinite):"<<endl;
    printQStringList("    ", configNames );


    configNames = invest_openapi::config_helpers::generateFileNamesUpToRoot( QString("app.config")
                                                                           , QString("conf;config")
                                                                           , QString(QCoreApplication::applicationDirPath())
                                                                           , false  // useSubfoldersFirst
                                                                           , false  // skipNonexistentDirs
                                                                           );
    cout<<"Generated names (all, subfolders last, lookup level infinite):"<<endl;
    printQStringList("    ", configNames );


    configNames = invest_openapi::config_helpers::generateFileNamesUpToRoot( QString("app.config")
                                                                           , QString("conf;config")
                                                                           , QString(QCoreApplication::applicationDirPath())
                                                                           , true  // useSubfoldersFirst
                                                                           , true  // skipNonexistentDirs
                                                                           );
    cout<<"Generated names (existent subfolders, subfolders first, lookup level infinite):"<<endl;
    printQStringList("    ", configNames );


    configNames = invest_openapi::config_helpers::generateFileNamesUpToRoot( QString("app.config")
                                                                           , QString("conf;config")
                                                                           , QString(QCoreApplication::applicationDirPath())
                                                                           , false  // useSubfoldersFirst
                                                                           , true   // skipNonexistentDirs
                                                                           );
    cout<<"Generated names (existent subfolders, subfolders last, lookup level infinite):"<<endl;
    printQStringList("    ", configNames );
   

    configNames = invest_openapi::config_helpers::generateFileNamesUpToRoot( QString("app.config")
                                                                           , QString("conf;config")
                                                                           , QString(QCoreApplication::applicationDirPath())
                                                                           , true  // useSubfoldersFirst
                                                                           , true  // skipNonexistentDirs
                                                                           , 1
                                                                           );
    cout<<"Generated names (existent subfolders, subfolders first, lookup level 1):"<<endl;
    printQStringList("    ", configNames );


    configNames = invest_openapi::config_helpers::generateFileNamesUpToRoot( QString("app.config")
                                                                           , QString("conf;config")
                                                                           , QString(QCoreApplication::applicationDirPath())
                                                                           , false  // useSubfoldersFirst
                                                                           , true   // skipNonexistentDirs
                                                                           , 1
                                                                           );
    cout<<"Generated names (existent subfolders, subfolders last, lookup level 1):"<<endl;
    printQStringList("    ", configNames );
   

    return 0;
}



