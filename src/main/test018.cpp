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
#include "invest_openapi/qt_time_helpers.h"





INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("test018");
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


    auto logConfigFullFileName   = lookupForConfigFile( "logging.properties" , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto apiConfigFullFileName   = lookupForConfigFile( "config.properties"  , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto authConfigFullFileName  = lookupForConfigFile( "auth.properties"    , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );

    qDebug().nospace().noquote() << "Log  Config File: "<< logConfigFullFileName  ;
    qDebug().nospace().noquote() << "API  Config File: "<< apiConfigFullFileName  ;
    qDebug().nospace().noquote() << "Auth Config File: "<< authConfigFullFileName ;

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

    // На левую дату приходит пустой ответ без ошибок

    QDate currentDate = QDate::currentDate();
    QDate startDate   = QDate::fromString("2011-12-19", Qt::ISODate);


    bool bFound = false;
    QDateTime foundDateTime;

    while(startDate < currentDate)
    {
        QDate nextDate = qt_helpers::addYearsNotGreaterThanDate( startDate, 10, currentDate );

        QDateTime requestBeginTime; requestBeginTime.setDate( startDate );
        QDateTime requestEndTime  ; requestEndTime  .setDate( nextDate );

        QElapsedTimer timer;
        timer.start();

        // BBG004731354 - ROSN

        auto // CandlesResponse
        candlesRes = pOpenApi->marketCandles( "BBG004731354", requestBeginTime, requestEndTime, "MONTH" );

        candlesRes->join();

        auto timeElapsed = timer.restart();
        qDebug().nospace().noquote() << "Time elapsed: " << timeElapsed;

        tkf::checkAbort(candlesRes);

        auto candles = candlesRes->value.getPayload();
        QList<tkf::Candle> candleList = candles.getCandles();

        for( const auto &candle : candleList)
        {
            if (!candle.isSet() || !candle.isValid())
            {
                 qDebug().nospace().noquote() << "Not valid candle, continue";
                 continue;
            }
       
            QDateTime candleDt = candle.getTime();

            // Не факт, что данные отсортированы по дате

            if (!bFound)
            {
                bFound = true;
                foundDateTime = candleDt;
            }
            else
            {
                if (foundDateTime>candleDt)
                    foundDateTime = candleDt;
            }
        }

        // Найдено на первом 10ти-летнем интервале
        // Пока не актуально, так как биржа MOEX работает меньше 10ти лет, но в 2021-12-19 10 лет закончатся.
        if (bFound)
            break;

        startDate = nextDate;

        qDebug().nospace().noquote() << "Continuing";

    }

    if (!bFound)
    {
        qDebug().nospace().noquote() << "Not found starting date";
        return 0;
    }

    qDebug().nospace().noquote() << "Starting date (month): " << qt_helpers::dateTimeToDbString(foundDateTime) ;


    bFound = false;


    startDate = foundDateTime.date();

    if ( !startDate.setDate( startDate.year(), startDate.month(), 1) )
    {
        qDebug().nospace().noquote() << "Failed to reset date to first day of month";
        return 0;
    }

    QDate nextDate = startDate.addMonths(1);

    QDateTime requestBeginTime; requestBeginTime.setDate( startDate );
    QDateTime requestEndTime  ; requestEndTime  .setDate( nextDate );


    auto // CandlesResponse
    candlesRes = pOpenApi->marketCandles( "BBG004731354", requestBeginTime, requestEndTime, "DAY" );

    candlesRes->join();
    tkf::checkAbort(candlesRes);

    auto candles = candlesRes->value.getPayload();
    QList<tkf::Candle> candleList = candles.getCandles();

    for( const auto &candle : candleList)
    {
        if (!candle.isSet() || !candle.isValid())
             continue;
    
        QDateTime candleDt = candle.getTime();

        // Не факт, что данные отсортированы по дате

        if (!bFound)
        {
            bFound = true;
            foundDateTime = candleDt;
        }
        else
        {
            if (foundDateTime>candleDt)
                foundDateTime = candleDt;
        }
    }


    if (!bFound)
    {
        qDebug().nospace().noquote() << "Exact date not found date";
        return 0;
    }

    qDebug().nospace().noquote() << "Starting date (exact): " << qt_helpers::dateTimeToDbString(foundDateTime) ;
    
    return 0;
}



