/*! \file
    \brief 
 */

#include <iostream>
#include <exception>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <optional>
#include <atomic>

#include <QCoreApplication>
#include <QString>
#include <QSettings>
#include <QTest>
#include <QDir>
#include <QElapsedTimer>
#include <QtWebSockets>

#include "invest_openapi/config_helpers.h"
#include "invest_openapi/api_config.h"
#include "invest_openapi/auth_config.h"
#include "invest_openapi/currencies_config.h"
#include "invest_openapi/balance_config.h"

#include "invest_openapi/invest_openapi.h"
#include "invest_openapi/factory.h"
#include "invest_openapi/openapi_completable_future.h"

#include "invest_openapi/database_config.h"
#include "invest_openapi/database_manager.h"
#include "invest_openapi/qt_time_helpers.h"

#include "invest_openapi/db_utils.h"
#include "invest_openapi/ioa_utils.h"
#include "invest_openapi/ioa_ostream.h"
#include "invest_openapi/ioa_db_dictionaries.h"

#include "invest_openapi/console_break_helper.h"


#include "invest_openapi/streaming_handlers.h"

#include "invest_openapi/market_glass.h"
#include "invest_openapi/market_instrument_state.h"




INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("_tkf_get_operations");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;

    cout<<"# Path to exe   : "<<QCoreApplication::applicationDirPath().toStdString()<<endl;

    cout << "#" << endl;

    namespace tkf=invest_openapi;
    using tkf::config_helpers::lookupForConfigFile;
    using tkf::config_helpers::FileReadable;

    QStringList lookupConfSubfolders = QString("conf;config").split( ';', Qt::SkipEmptyParts );

    auto logConfigFullFileName     = lookupForConfigFile( "logging.properties" , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto apiConfigFullFileName     = lookupForConfigFile( "config.properties"  , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto authConfigFullFileName    = lookupForConfigFile( "auth.properties"    , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto dbConfigFullFileName      = lookupForConfigFile( "database.properties", lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto instrumentsConfigFullFileName = lookupForConfigFile( "instruments.properties" , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );

    cout << "# Log  Config File: "<< logConfigFullFileName   << endl;
    cout << "# API  Config File: "<< apiConfigFullFileName   << endl;
    cout << "# Auth Config File: "<< authConfigFullFileName  << endl;
    cout << "# Instruments Cfg File: "<< instrumentsConfigFullFileName << endl;

    auto apiConfig     = tkf::ApiConfig    ( apiConfigFullFileName  );
    auto authConfig    = tkf::AuthConfig   ( authConfigFullFileName );

    QSharedPointer<tkf::DatabaseConfig> pDatabaseConfig = QSharedPointer<tkf::DatabaseConfig>( new tkf::DatabaseConfig(dbConfigFullFileName, tkf::DatabasePlacementStrategyDefault()) );

    QSharedPointer<tkf::LoggingConfig>  pLoggingConfig  = QSharedPointer<tkf::LoggingConfig> ( new tkf::LoggingConfig(logConfigFullFileName) );

    // Force turn of requests logging
    pLoggingConfig->debugSqlQueries = false;
    pLoggingConfig->debugRequests   = false;
    pLoggingConfig->debugResponses  = false;


    auto loggingConfig = *pLoggingConfig;



    auto dataLogFullFilename = pLoggingConfig->getDataLogFullName( logConfigFullFileName, "", "test.dat" );

    cout << "# test_streaming_api data log file: "<< dataLogFullFilename << endl;




    cout << "# Main DB name: " << pDatabaseConfig->dbMainFilename << endl;

    QSharedPointer<QSqlDatabase> pMainSqlDb = QSharedPointer<QSqlDatabase>( new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE")) );
    pMainSqlDb->setDatabaseName( pDatabaseConfig->dbMainFilename );

    if (!pMainSqlDb->open())
    {
      //qDebug() 
      cout << pMainSqlDb->lastError().text() << endl;
      return 0;
    }

    QSharedPointer<tkf::IDatabaseManager> pMainDbMan = tkf::createMainDatabaseManager( pMainSqlDb, pDatabaseConfig, pLoggingConfig );

    pMainDbMan->applyDefDecimalFormatFromConfig( *pDatabaseConfig );



    QSharedPointer<tkf::IOpenApi> pOpenApi = tkf::createOpenApi( apiConfig, authConfig, loggingConfig );

    tkf::ISanboxOpenApi* pSandboxOpenApi = tkf::getSandboxApi(pOpenApi);

    if (pSandboxOpenApi)
    {
        pSandboxOpenApi->setBrokerAccountId( authConfig.getBrokerAccountId() );
    }
    else
    {
        pOpenApi->setBrokerAccountId( authConfig.getBrokerAccountId() );
    }


    tkf::DatabaseDictionaries dicts = tkf::DatabaseDictionaries(pMainDbMan);



    QStringList instrumentList;
    {
        QSettings settings(instrumentsConfigFullFileName, QSettings::IniFormat);
        instrumentList = settings.value("instruments" ).toStringList();
    }


    for( auto instrument : instrumentList )
    {
        QString figi   = dicts.findFigiByAnyIdString(instrument);
        QString ticker = dicts.getTickerByFigiChecked(figi);

        cout << "-------------------------------" << endl;

        cout << "Try to get operations on " << ticker << " (" << figi << ")" << endl;

        QDateTime dateTimeNow     = QDateTime::currentDateTime();
        QDateTime dateTimeBefore  = qt_helpers::dtAddTimeInterval( dateTimeNow, QString("-10YEAR") );
        
        // const QDateTime &from, const QDateTime &to, const QString &figi, QString broker_account_id
        auto operationsResponse = pOpenApi->operations(dateTimeBefore, dateTimeNow, figi);

        operationsResponse->join();

        tkf::checkAbort(operationsResponse);

        // QList<Operation> getOperations() const;
        auto operations = operationsResponse->value.getPayload().getOperations();

        for( auto op : operations )
        {
            // QList<OperationTrade> op.getTrades() const;
            // OperationTypeWithCommission getOperationType() const;

            QString operationTypeStr   = op.getOperationType().asJson().toUpper();
            QString operationStatusStr = op.getStatus().asJson().toUpper();

            if (operationStatusStr!="DONE" && operationStatusStr!="PROGRESS") // DECLINE or INVALID
                continue;

            if (operationTypeStr!="BUYCARD" && operationTypeStr!="BUY" && operationTypeStr!="SELL")
                continue;

            cout << "Operation: " << operationTypeStr << endl;
            cout << "Status   : " << operationStatusStr << endl;
            cout << "Trades   :"  << endl;


            auto trades = op.getTrades(); // QList<OperationTrade>

            for( auto trade : trades )
            {
                cout << "    " << trade.getQuantity() << " x " << trade.getPrice() << endl;
            }

            cout << endl;

        }

    }


    // Custom code goes here

    
    return 0;
}



