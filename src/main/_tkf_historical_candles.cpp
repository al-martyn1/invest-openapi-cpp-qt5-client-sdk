/*! \file
    \brief Configs lookup test

 */

#include <iostream>
#include <exception>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <map>
#include <set>
#include <optional>

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
#include "invest_openapi/balance_config.h"

#include "invest_openapi/invest_openapi.h"
#include "invest_openapi/factory.h"
#include "invest_openapi/openapi_completable_future.h"

#include "invest_openapi/database_config.h"
#include "invest_openapi/database_manager.h"
#include "invest_openapi/qt_time_helpers.h"

#include "invest_openapi/db_utils.h"
#include "invest_openapi/qt_time_helpers.h"
#include "invest_openapi/ioa_utils.h"
#include "invest_openapi/ioa_ostream.h"
#include "invest_openapi/ioa_db_dictionaries.h"

#include "invest_openapi/console_break_helper.h"

INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("_tkf_historical_candles");
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


    auto logConfigFullFileName     = lookupForConfigFile( "logging.properties" , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto apiConfigFullFileName     = lookupForConfigFile( "config.properties"  , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto authConfigFullFileName    = lookupForConfigFile( "auth.properties"    , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto dbConfigFullFileName      = lookupForConfigFile( "database.properties", lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    //auto balanceConfigFullFileName = lookupForConfigFile( "balance.properties" , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );

    qDebug().nospace().noquote() << "Log  Config File: "<< logConfigFullFileName  ;
    qDebug().nospace().noquote() << "API  Config File: "<< apiConfigFullFileName  ;
    qDebug().nospace().noquote() << "Auth Config File: "<< authConfigFullFileName ;
    qDebug().nospace().noquote() << "DB   Config     : "<< dbConfigFullFileName   ;
    //qDebug().nospace().noquote() << "Balance Config  : "<< balanceConfigFullFileName;

    auto apiConfig     = tkf::ApiConfig    ( apiConfigFullFileName  );
    auto authConfig    = tkf::AuthConfig   ( authConfigFullFileName );
    //auto balanceConfig = tkf::BalanceConfig( balanceConfigFullFileName );
    //auto balance_config.h


    QSharedPointer<tkf::DatabaseConfig> pDatabaseConfig = QSharedPointer<tkf::DatabaseConfig>( new tkf::DatabaseConfig(dbConfigFullFileName, tkf::DatabasePlacementStrategyDefault()) );
    QSharedPointer<tkf::LoggingConfig>  pLoggingConfig  = QSharedPointer<tkf::LoggingConfig> ( new tkf::LoggingConfig(logConfigFullFileName) );
    auto loggingConfig = *pLoggingConfig;


    qDebug().nospace().noquote() << "DB name: " << pDatabaseConfig->dbFilename;

    QSharedPointer<QSqlDatabase> pSqlDb = QSharedPointer<QSqlDatabase>( new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE")) );
    pSqlDb->setDatabaseName( pDatabaseConfig->dbFilename );

    if (!pSqlDb->open())
    {
      qDebug() << pSqlDb->lastError().text();
      return 0;
    }

    QSharedPointer<tkf::IDatabaseManager> pDbMan = tkf::createDatabaseManager( pSqlDb, pDatabaseConfig, pLoggingConfig );

    pDbMan->applyDefDecimalFormatFromConfig( *pDatabaseConfig );



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

    tkf::DatabaseDictionaries dicts = tkf::DatabaseDictionaries(pDbMan);


    int stockExchangeId = dicts.getStockExangeListIdChecked("moex");

    int instrumentId     = dicts.getInstrumentIdBegin( );
    int instrumentIdEnd  = dicts.getInstrumentIdEnd( );


    console_helpers::SimpleHandleCtrlC ctrlC;



    for( ; instrumentId!=instrumentIdEnd && !ctrlC.isBreaked(); ++instrumentId )
    {
        QString figi = dicts.getInstrumentById( instrumentId );
        if (figi.isEmpty())
            continue; // There is a GAP found in instruments enumeration

        QString ticker         = dicts.getTickerByFigiChecked(figi);
        QString instrumentName = dicts.getNameByFigiChecked(figi);

        

        cout << endl << "----------------------------------------------------------------------------" << endl;
        cout << "Processing " << figi << " (" << ticker << ") - " << instrumentName << endl;

        int candleResolutionId     = dicts.getCandleResolutionIdBegin( );
        int candleResolutionIdEnd  = dicts.getCandleResolutionIdEnd( );

        for(; candleResolutionId!=candleResolutionIdEnd && !ctrlC.isBreaked(); ++candleResolutionId)
        {
            QString candleResolution = dicts.getCandleResolutionById(candleResolutionId);
            if (!dicts.isValidId(candleResolution))
               continue;

            QString candleIntervalMin = dicts.getCandleResolutionIntervalMinById(candleResolutionId);
            QString candleIntervalMax = dicts.getCandleResolutionIntervalMaxById(candleResolutionId);

            cout << "    " << "Processing candles with resolution " << candleResolution 
                 << ", interval min: " << candleIntervalMin 
                 << ", interval max: " << candleIntervalMax << endl;

            QElapsedTimer candleLastDateLookupTimer;
            candleLastDateLookupTimer.start();


            /*
            SELECT column_list FROM table_list
            WHERE row_filter
            ORDER BY column
            LIMIT count OFFSET offset
            ORDER BY
            column_1 ASC,
            column_2 DESC;
            */

            /*
             Для проверки
             SELECT INSTRUMENT_FIGI, INSTRUMENT_TICKER, LISTING_DATE FROM INSTRUMENT_LISTING_DATES
             ORDER BY LISTING_DATE DESC
             LIMIT 20 OFFSET 0
             */

            // Нужно найти дату последней имеющейся свечи
            // INSTRUMENT_CANDLES, поля INSTRUMENT_ID, STOCK_EXCHANGE_ID, CANDLE_RESOLUTION_ID - фильтр
            // Выбираем поле CANDLE_DATE_TIME с максимальным значением

            // QVector{ instrumentId, stockExchangeId, candleResolutionId }
            // QString::number(instrumentId)

            QString selectQueryText 
                        = pDbMan->makeSimpleSelectQueryText( "INSTRUMENT_CANDLES"
                                                           , "INSTRUMENT_ID,STOCK_EXCHANGE_ID,CANDLE_RESOLUTION_ID" // whereNames
                                                           , QString("%1,%2,%3").arg(instrumentId).arg(stockExchangeId).arg(candleResolutionId) // whereVals
                                                           , "CANDLE_DATE_TIME" // fields to select
                                                           );

            QString selectLastDateQueryText 
                        = pDbMan->makeSelectSingleDateQuery( selectQueryText, "CANDLE_DATE_TIME", true /* true for last, false for first */ );


            bool papyNotGood = true;

            QDateTime dtLastCandleDate;

            // std::optional - https://habr.com/ru/post/372103/
            // Но на самом деле он не нужен

            auto resVec = pDbMan->execSelectQueryReturnFirstRow( selectLastDateQueryText );

            if ( !resVec.empty() && !resVec.front().isEmpty() )
            {
                //cout << "Starting date found in INSTRUMENT_CANDLES" << endl;
                dtLastCandleDate = qt_helpers::dateTimeFromDbString( resVec.front() );
            }



            if (!tkf::isQtValidNotNull(dtLastCandleDate))
            {
                // Если дата не найдена в таблице, ищем дату листинга инструмента на бирже
                // INSTRUMENT_LISTING_DATES, поля INSTRUMENT_ID, STOCK_EXCHANGE_ID - фильтр
                // Выбираем поле LISTING_DATE

                cout << "Looking in INSTRUMENT_LISTING_DATES" << endl;

                selectQueryText 
                        = pDbMan->makeSimpleSelectQueryText( "INSTRUMENT_LISTING_DATES"
                                                           , "INSTRUMENT_ID,STOCK_EXCHANGE_ID" // whereNames
                                                           , QString("%1,%2").arg(instrumentId).arg(stockExchangeId) // whereVals
                                                           , "LISTING_DATE" // fields to select
                                                           );

                selectLastDateQueryText 
                        = pDbMan->makeSelectSingleDateQuery( selectQueryText, "LISTING_DATE", true /* true for last, false for first */ );


                auto resVec = pDbMan->execSelectQueryReturnFirstRow( selectLastDateQueryText );

                if ( !resVec.empty() && !resVec.front().isEmpty() )
                {
                    auto date = qt_helpers::dateFromDbString( resVec.front() );
                    auto zeroTime = QTime(0 /* h */, 0 /* m */ , 0 /* s */, 0 /* ms */ );
                    dtLastCandleDate = QDateTime(date, zeroTime, Qt::UTC); // .setDate(date);
                }

            }

            auto elapsed = (unsigned)candleLastDateLookupTimer.restart();

            cout << "    " << "DB queries performed in " << elapsed << " ms" << endl;

            if (!tkf::isQtValidNotNull(dtLastCandleDate))
            {
                cout << "    " << "Starting date not found at all" << endl;
                continue;
            }


            // Ду Жоб Хере

            cout << "    " << "Starting date: " << dtLastCandleDate << endl;

            QDateTime curDateTime = QDateTime::currentDateTime().toUTC();

            std::vector< QDateTime > candleIntervals;

            QDateTime dateTime = qt_helpers::makeMidnightDateTime(dtLastCandleDate);

            candleIntervals.push_back(dateTime);

            while(true)
            {
                QDateTime nextDateTime = qt_helpers::dtAddTimeInterval(dateTime, candleIntervalMax);

                //cout << "    " << "Next: " << nextDateTime << endl;

                if (nextDateTime>curDateTime)
                {
                    nextDateTime = curDateTime;
                    candleIntervals.push_back(nextDateTime);
                    break;
                }

                candleIntervals.push_back(nextDateTime);

                dateTime = nextDateTime;

            }

            std::vector< QDateTime >::const_iterator intervalIt    = candleIntervals.begin();
            std::vector< QDateTime >::const_iterator intervalItEnd = candleIntervals.end();

            if (intervalIt==intervalItEnd)
            {
                continue;
            }

            std::vector< QDateTime >::const_iterator curIntervalItBegin = intervalIt;
            ++intervalIt;
            std::vector< QDateTime >::const_iterator curIntervalItEnd   = intervalIt;

            for( ; curIntervalItEnd!=intervalItEnd; ++curIntervalItBegin, ++curIntervalItEnd )
            {
                cout << "    " << *curIntervalItBegin << " - " << *curIntervalItEnd << endl;


            }


            // qt_helpers::makeMidnightDateTime( const QDate &date )
            // qt_helpers::makeMidnightDateTime( const QDateTime &dt )
            // QString candleIntervalMax = dicts.getCandleResolutionIntervalMaxById(candleResolutionId);


            //QDateTime qt_helpers::dtAddTimeInterval( const QDateTime &dt, QString intervalStr, int forceSign = 0 /* 0 - use default */ )






        }


    }


    
    return 0;
}



