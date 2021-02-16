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
    //QDate reqDate = QDate::fromString("2001-02-11", Qt::ISODate);

    //QDate     curDate          = QDate::currentDate();
    QDate reqDate = QDate::fromString("2021-02-11", Qt::ISODate);
    QDateTime requestBeginTime; requestBeginTime.setDate(reqDate /* .addDays(0) */ );
    QDateTime requestEndTime  ; requestEndTime  .setDate(reqDate.addDays(1));

    //QDate::currentDate()
    // BBG004731354 - ROSN
    auto // CandlesResponse
    candlesRes = pOpenApi->marketCandles( "BBG004731354", requestBeginTime, requestEndTime, "5MIN" );

    candlesRes->join();
    tkf::checkAbort(candlesRes);

    // candlesRes->value.getStatus()
    // candlesRes->value.getTrackingId()

    auto candles = candlesRes->value.getPayload();
    QList<tkf::Candle> candleList = candles.getCandles();

    for( const auto &candle : candleList)
    {
        if (!candle.isSet() || !candle.isValid())
             continue;

        QDateTime candleDt = candle.getTime();

        qDebug().nospace().noquote() << "FIGI: " << candle.getFigi() << ", Candle DateTime: " << qt_helpers::formatDateTimeISO8601(candleDt);
    
    }

    
    return 0;
}



