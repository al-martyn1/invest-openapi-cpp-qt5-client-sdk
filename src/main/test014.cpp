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
#include "invest_openapi/currencies_config.h"

#include "invest_openapi/invest_openapi.h"
#include "invest_openapi/factory.h"
#include "invest_openapi/openapi_completable_future.h"
#include "invest_openapi/model_to_strings.h"


INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("test014");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;


    cout<<"Launched exe  : "<<QCoreApplication::applicationFilePath().toStdString()<<endl;

    cout << endl;

    namespace tkf=invest_openapi;
    using tkf::config_helpers::lookupForConfigFile;
    using tkf::config_helpers::FileReadable;

    QSet<QString>         allSqlTables  = tkf::modelMakeAllSqlTablesSet();
    QMap<QString,QString> allSqlSchemas = tkf::modelMakeAllSqlShemas();

    for( auto sqlTable : allSqlTables )
    {
        auto it = allSqlSchemas.find(sqlTable);
        if (it==allSqlSchemas.end())
        {
            cout << "Table '" << sqlTable.toStdString() << "' not found in schemes" << endl << endl;
        }
        else
        {
            QString key = *it;
            cout << "Table '" << sqlTable.toStdString() << "' schema:" << endl;
            //cout << it->second << endl << endl;
            cout << allSqlSchemas[key].toStdString() << endl << endl;
        }
    }


    

    
    return 0;
}



