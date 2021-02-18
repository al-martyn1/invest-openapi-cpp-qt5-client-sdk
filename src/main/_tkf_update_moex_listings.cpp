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
#include "invest_openapi/database_config.h"


#include "invest_openapi/invest_openapi.h"
#include "invest_openapi/factory.h"
#include "invest_openapi/openapi_completable_future.h"
#include "invest_openapi/i_database_manager.h"
#include "invest_openapi/database_manager.h"

#include "invest_openapi/model_to_strings.h"



INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("_tkf_update_moex_listings");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;


    cout<<"Path to exe   : "<<QCoreApplication::applicationDirPath().toStdString()<<endl;

    cout << endl;

    namespace tkf=invest_openapi;
    using tkf::config_helpers::lookupForConfigFile;
    using tkf::config_helpers::FileReadable;

    QStringList lookupConfSubfolders = QString("conf;config").split( ';', Qt::SkipEmptyParts );


    auto dbConfigFullFileName    = lookupForConfigFile( "database.properties", lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto logConfigFullFileName   = lookupForConfigFile( "logging.properties" , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto apiConfigFullFileName   = lookupForConfigFile( "config.properties"  , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto authConfigFullFileName  = lookupForConfigFile( "auth.properties"    , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );

    qDebug().nospace().noquote() << "DB   Config File: "<< dbConfigFullFileName   ;
    qDebug().nospace().noquote() << "Log  Config File: "<< logConfigFullFileName  ;
    qDebug().nospace().noquote() << "API  Config File: "<< apiConfigFullFileName  ;
    qDebug().nospace().noquote() << "Auth Config File: "<< authConfigFullFileName ;


    QSharedPointer<tkf::DatabaseConfig> pDatabaseConfig = QSharedPointer<tkf::DatabaseConfig>( new tkf::DatabaseConfig(dbConfigFullFileName, tkf::DatabasePlacementStrategyDefault()) );
    QSharedPointer<tkf::LoggingConfig>  pLoggingConfig  = QSharedPointer<tkf::LoggingConfig> ( new tkf::LoggingConfig(logConfigFullFileName) );

    qDebug().nospace().noquote() << "DB name: " << pDatabaseConfig->dbFilename;

    QSharedPointer<QSqlDatabase> pSqlDb = QSharedPointer<QSqlDatabase>( new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE")) );
    pSqlDb->setDatabaseName( pDatabaseConfig->dbFilename );

    if (!pSqlDb->open())
    {
        qDebug() << pSqlDb->lastError().text();
        return 0;
    }

    QSharedPointer<tkf::IDatabaseManager> pDbMan = tkf::createDatabaseManager( pSqlDb, pDatabaseConfig, pLoggingConfig );
    pDbMan->applyDefDecimalFromConfig( *pDatabaseConfig );


    auto apiConfig     = tkf::ApiConfig    ( apiConfigFullFileName  );
    auto authConfig    = tkf::AuthConfig   ( authConfigFullFileName );
    auto loggingConfig = tkf::LoggingConfig( logConfigFullFileName  );

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


    //----------------------------------------------------------------------------
    QVector<QString> instrumentListingDatesTableColumns = pDbMan->tableGetColumnsFromSchema  ( "INSTRUMENT_LISTING_DATES" );

    //qDebug().nospace().noquote() << "INSTRUMENT_LISTING_DATES: " << instrumentListingDatesTableColumns;

    //----------------------------------------------------------------------------
    QString    selectMoexQueryText = pDbMan->makeSimpleSelectQueryText( "STOCK_EXCHANGE_LIST", "NAME", "MOEX", "ID;NAME;FOUNDATION_DATE" );

    bool queryRes = false;
    QSqlQuery  moexExecutedQuery   = pDbMan->execHelper ( selectMoexQueryText, &queryRes );

    if (!queryRes)
    {
        qDebug().nospace().noquote() << "Select MOEX failed: " << moexExecutedQuery.lastError().text();
        return 1;
    }

    QVector< QVector<QString> > vvMoex = pDbMan->selectResultToStringVectors( moexExecutedQuery );
    if (vvMoex.empty())
    {
        qDebug().nospace().noquote() << "Select MOEX failed: no such entry";
        return 1;
    }

    QVector<QString> vMoexIdName = vvMoex[0];

    qDebug().nospace().noquote() << "MOEX foundation date (as string): " << vMoexIdName[2];
    QDate moexFoundationDate = qt_helpers::dateFromDbString(vMoexIdName[2]);
    qDebug().nospace().noquote() << "MOEX foundation date (as date)  : " << moexFoundationDate;
    vMoexIdName.remove(2);

    qDebug().nospace().noquote() << "MOEX: " << vMoexIdName;


    QString    selectFigiQueryText = pDbMan->makeSimpleSelectQueryText( "MARKET_INSTRUMENT", "ID;FIGI;TICKER" );
    QSqlQuery  figiExucutedQuery   = pDbMan->execHelper ( selectFigiQueryText, &queryRes );

    QVector< QVector<QString> > vvFigis = pDbMan->selectResultToStringVectors(figiExucutedQuery);

    // int counter = 0; // Only first 10 items for first time

    QElapsedTimer firstFailElapsedTimer; 
    QElapsedTimer failElapsedTimer; 
    QElapsedTimer globalElapsedTimer;
    QElapsedTimer timer;

    firstFailElapsedTimer.start();
    failElapsedTimer.start();
    globalElapsedTimer.start();
    timer.start();

    unsigned firstFailElapsedTime = 0;
    unsigned failElapsedTime = 0;
    unsigned elapsedTime = 0;

    unsigned totalCounter = 0;
    unsigned failCounter = 0;

    for( auto figiRow: vvFigis )
    {
        // 134 записи проверяются за 35 секунд, что привышает лимиты на запросы
        QTest::qWait(1000); // Поэтому добавляем по секунде ожидания на каждую запись, чтобы гарантировано уложиться в лимит 120 запросов в минуту
        // по секунде - потому чтобыу нас как минимум два запроса на каждую фигу

        // figiRow contains ID;FIGI;TICKER from MARKET_INSTRUMENT
        // vMoexIdName contains ID;NAME from STOCK_EXCHANGE_LIST
        QString checkListingDateExistQuery = pDbMan->makeSimpleSelectQueryText( "INSTRUMENT_LISTING_DATES"
                                                                              , QVector<QString>{ "INSTRUMENT_ID", "STOCK_EXCHANGE_ID" }
                                                                              , QVector<QString>{ figiRow[0], vMoexIdName[0] }
                                                                              );
        timer.restart();
        QSqlQuery  checkListingDateExistExecutedQuery = pDbMan->execHelper ( checkListingDateExistQuery, &queryRes );
        std::size_t foundRecordsNumber = pDbMan->getQueryResultSize( checkListingDateExistExecutedQuery, true  /* needBool */ );
        elapsedTime = (unsigned)timer.restart();

        if (foundRecordsNumber)
        {
            qDebug().nospace().noquote() << "Listing start date for " << figiRow[1] << " (" << figiRow[2] << ") already exist for " << vMoexIdName[1] << ", elapsed time: " << elapsedTime;
            continue;
        }
        
        qDebug().nospace().noquote() << "Looking up for listing start date for: " << figiRow[1] << " (" << figiRow[2] << ")";

        // counter++; // Only first 10 items for first time


        QDate foundDate;
        tkf::GenericError lookupRes = tkf::GenericError::noData;

        std::size_t tryNo = 0, maxTries = 10;

        for(; tryNo!=maxTries; ++tryNo)
        {
            timer.restart();

            lookupRes = pOpenApi->findInstrumentListingStartDate( figiRow[1], moexFoundationDate, foundDate );

            elapsedTime = (unsigned)timer.restart();

            if (lookupRes!=tkf::GenericError::networkError)
                break;

            if (!failCounter)
            {
                firstFailElapsedTime = (unsigned)firstFailElapsedTimer.elapsed();
                qDebug().nospace().noquote() << "First fail occurs on " << firstFailElapsedTime << " ms from start, processed item: #" << (totalCounter+1);
            }

            failCounter++;

            qDebug().nospace().noquote() << "Unsuccessful attempt #" << tryNo << ", elapsed time: " << elapsedTime;

            QTest::qWait(1000*(tryNo+1));
        }

        /*
        if (lookupRes)
        {
            qDebug().nospace().noquote() << "Lookup failed on " << figiRow[1] << " (" << figiRow[2] << ")"; // , elapsed time: " << elapsedTime;
            failElapsedTime = (unsigned)failElapsedTimer.elapsed();
            qDebug().nospace().noquote() << "Failed on " << failElapsedTime << " ms from start";

            return 1;
        }
        */
/*
enum class GenericError
{
    ok,
    internalError,
    networkError,
    noData
};

*/

        if (lookupRes==tkf::GenericError::internalError)
        {
            qDebug().nospace().noquote() << "Lookup listing start date failed due internal error, elapsed time: " << elapsedTime;
        }
        else if (lookupRes==tkf::GenericError::networkError)
        {
            qDebug().nospace().noquote() << "Lookup listing start date failed due network error, elapsed time: " << elapsedTime;
        }
        else if (lookupRes==tkf::GenericError::networkError)
        {
            qDebug().nospace().noquote() << "Lookup listing start date failed: no data, elapsed time: " << elapsedTime;
        }
        else
        {
            qDebug().nospace().noquote() << "Listing start date for " << figiRow[1] << " (" << figiRow[2] << ") was found, elapsed time: " << elapsedTime;

            // figiRow contains ID;FIGI;TICKER from MARKET_INSTRUMENT
            // vMoexIdName contains ID;NAME from STOCK_EXCHANGE_LIST
            // Inserting to INSTRUMENT_LISTING_DATES: INSTRUMENT_ID, INSTRUMENT_FIGI, INSTRUMENT_TICKER, STOCK_EXCHANGE_ID, STOCK_EXCHANGE_NAME, LISTING_DATE
           
            figiRow.append( vMoexIdName );
            figiRow.append( qt_helpers::dateToDbString(foundDate) );
           
            timer.restart();
            pDbMan->insertTo( "INSTRUMENT_LISTING_DATES", figiRow, instrumentListingDatesTableColumns );
            elapsedTime = (unsigned)timer.restart();
            qDebug().nospace().noquote() << "Listing start date for " << figiRow[1] << " (" << figiRow[2] << ") was inserted, elapsed time: " << elapsedTime;
           
            // if (counter>=10) break; // Only first 10 items for first time
        }
           
        totalCounter++;

    }

    elapsedTime = (unsigned)globalElapsedTimer.restart();

    elapsedTime -= 1000*totalCounter;


    qDebug().nospace().noquote() << "-------------------";
    qDebug().nospace().noquote() << "Done, time elapsed (clean): " << elapsedTime;


    return 0;
}



