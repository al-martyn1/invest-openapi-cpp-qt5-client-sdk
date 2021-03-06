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
    QCoreApplication::setApplicationName("test005");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;


    cout<<"Launched from : "<<QDir::currentPath().toStdString()<<endl;
    cout<<"Launched exe  : "<<QCoreApplication::applicationFilePath().toStdString()<<endl;
    cout<<"Path to exe   : "<<QCoreApplication::applicationDirPath().toStdString()<<endl;

    using invest_openapi::config_helpers::lookupForConfigFiles;

    QStringList founfStringList = lookupForConfigFiles( "config.properties"
                                                      , "conf;config"
                                                      , invest_openapi::config_helpers::FileReadable()
                                                      , QCoreApplication::applicationDirPath()
                                                      , false // useSubfoldersFirst
                                                      , 10 // maxUpLevel
                                                      );

    cout<<"Found config files:"<<endl;
    printQStringList("    ", founfStringList );

    
    return 0;
}



