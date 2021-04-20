/*! \file
    \brief Тесты парсинга StreamingAPI JSON ответов и генерации запросов

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
#include "invest_openapi/ioa_utils.h"
#include "invest_openapi/ioa_ostream.h"
#include "invest_openapi/ioa_db_dictionaries.h"

#include "invest_openapi/console_break_helper.h"


INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("test028");
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

    qDebug().nospace().noquote() << "Log  Config File: "<< logConfigFullFileName  ;
    qDebug().nospace().noquote() << "API  Config File: "<< apiConfigFullFileName  ;
    qDebug().nospace().noquote() << "Auth Config File: "<< authConfigFullFileName ;
    qDebug().nospace().noquote() << "DB   Config     : "<< dbConfigFullFileName   ;

    auto apiConfig     = tkf::ApiConfig    ( apiConfigFullFileName  );
    auto authConfig    = tkf::AuthConfig   ( authConfigFullFileName );


    QSharedPointer<tkf::DatabaseConfig> pDatabaseConfig = QSharedPointer<tkf::DatabaseConfig>( new tkf::DatabaseConfig(dbConfigFullFileName, tkf::DatabasePlacementStrategyDefault()) );
    QSharedPointer<tkf::LoggingConfig>  pLoggingConfig  = QSharedPointer<tkf::LoggingConfig> ( new tkf::LoggingConfig(logConfigFullFileName) );
    auto loggingConfig = *pLoggingConfig;



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


    QString jsonCandleSample = 
                            "{"
                              "\"payload\":"
                                "{"
                                    "\"o\":154.58,"
                                    "\"c\":154.6,"
                                    "\"h\":154.6,"
                                    "\"l\":154.58,"
                                    "\"v\":326,"
                                    "\"time\":\"2021-04-20T13:55:00Z\","
                                    "\"interval\":\"1min\","
                                    "\"figi\":\"BBG000BN56Q9\""
                                "},"
                              "\"event\":\"candle\","
                              "\"time\":\"2021-04-20T13:55:39.717311393Z\""
                            "}";

    QString jsonErrSample = "{"
                              "\"payload\":"
                                "{"
                                    "\"error\":\"Subscription orderbook:subscribe. Invalid depth 0. Should be 0 \u003c x \u003c 20\""
                                "},"
                              "\"event\":\"error\","
                              "\"time\":\"2021-04-19T19:48:36.923636138Z\""
                            "}";

    QString jsonSample    = "{"
                              "\"payload\":"
                                "{"
                                    "\"figi\":\"BBG000BN56Q9\","
                                    "\"depth\":20,"
                                    "\"bids\":[[154,62],[153.98,2],[153.96,14],[153.94,14],[153.92,41],[153.9,32],[153.86,2],[153.82,49],[153.8,22],[153.78,15],[153.76,35],[153.74,206],[153.7,93],[153.66,5],[153.64,14],[153.56,154],[153.54,200],[153.52,202],[153.5,109],[153.46,18]],"
                                    "\"asks\":[[154.1,77],[154.12,23],[154.14,34],[154.16,56],[154.18,10],[154.2,26],[154.22,65],[154.3,66],[154.32,317],[154.34,200],[154.36,48],[154.4,75],[154.46,35],[154.5,100],[154.52,240],[154.56,200],[154.58,200],[154.6,200],[154.64,200],[154.66,255]]"
                                "},"
                              "\"event\":\"orderbook\","
                              "\"time\":\"2021-04-16T18:13:02.737266511Z\""
                            "}";
    cout << "JSON:" << endl;
    cout << jsonSample << endl;
    cout << endl;

    
    std::string jsonSampleStdString = jsonSample.toStdString();

    QByteArray    qba   = QByteArray(jsonSampleStdString.data(), jsonSampleStdString.size());
    QJsonDocument doc   = QJsonDocument::fromJson(qba);
    QJsonObject  json   = doc.object();

    // payload
    // event
    // time

    QString payload;
    bool payload_isValid = ::OpenAPI::fromJsonValue( payload, json[QString("payload")]);
    bool payload_isSet   = !json[QString("payload")].isNull() && payload_isValid;

    QString eventName;
    bool eventName_isValid = ::OpenAPI::fromJsonValue( eventName, json[QString("event")]);
    bool eventName_isSet   = !json[QString("event")].isNull() && eventName_isValid;

    QDateTime dateTime;
    bool dateTime_isValid = ::OpenAPI::fromJsonValue( dateTime, json[QString("time")]);
    bool dateTime_isSet   = !json[QString("time")].isNull() && dateTime_isValid;

    cout << "Payload:" << endl;
    cout << payload << endl;
    cout << endl;

    cout << "Event:" << endl;
    cout << eventName << endl;
    cout << endl;

    cout << "Time:" << endl;
    cout << dateTime << endl;
    cout << endl;

    tkf::GenericStreamingResponse genericStreamingResponse;
    genericStreamingResponse.fromJson(jsonSample);

    cout << "GenericStreamingResponse test" << endl;
    cout << "Event: " << genericStreamingResponse.getEvent() << endl;
    cout << "Time : " << genericStreamingResponse.getTime() << endl;
    cout << endl;


    tkf::StreamingOrderbookResponse streamingOrderbookResponse;
    streamingOrderbookResponse.fromJson(jsonSample);

    cout << "StreamingOrderbookResponse test" << endl;
    cout << "Event: " << streamingOrderbookResponse.getEvent() << endl;
    cout << "Time : " << streamingOrderbookResponse.getTime() << endl;
    cout << endl;


    tkf::StreamingOrderbook streamingOrderbook = streamingOrderbookResponse.getPayload();
    cout << "FIGI  : " << streamingOrderbook.getFigi()  << endl;
    cout << "Depth : " << streamingOrderbook.getDepth() << endl;

    QList< QList<marty::Decimal> > bids = streamingOrderbook.getBids();
    QList< QList<marty::Decimal> > asks = streamingOrderbook.getAsks();

    cout << "Bids:" << endl;
    for( auto bidLst : bids )
    {
        auto bid = tkf::StreamingOrderbookItem::fromList( bidLst );
        cout << "    " << bid.price << " x " << bid.quantity << endl;
    }

    cout << "Asks:" << endl;
    for( auto askLst : asks )
    {
        auto ask = tkf::StreamingOrderbookItem::fromList( askLst );
        cout << "    " << ask.price << " x " << ask.quantity << endl;
    }


    cout << endl;
    cout << "Streaming error JSON: " << jsonErrSample << endl;


    tkf::StreamingError streamingError;
    streamingError.fromJson(jsonErrSample);

    cout << "StreamingError test" << endl;
    cout << "StreamingError Event: " << streamingError.getEvent() << endl;
    cout << "StreamingError Time : " << streamingError.getTime() << endl;
    cout << "StreamingError Text : " << streamingError.getPayload().getMessage() << endl;
    cout << endl << endl;


    tkf::StreamingCandleResponse  streamingCandleResponse;
    streamingCandleResponse.fromJson(jsonCandleSample);
    cout << "StreamingCandleResponse test" << endl;
    cout << "StreamingCandleResponse Event : " << streamingCandleResponse.getEvent() << endl;
    cout << "StreamingCandleResponse Time  : " << streamingCandleResponse.getTime() << endl;
    cout << "StreamingCandleResponse Candle: " << tkf::modelToStrings( streamingCandleResponse.getPayload() ) << endl;
    cout << endl << endl;
    


    auto showJsonWithTitle = []( const QString &title, const QString &json )
             {
                 using std::cout;
                 using std::endl;

                 cout << title << ":" << endl << json << endl << endl;
             
             };

    showJsonWithTitle( "Orderbook subscribe"  , pOpenApi->getStreamingApiOrderbookSubscribeJson  ("BBG000BN56Q9", 20) );
    showJsonWithTitle( "Orderbook unsubscribe", pOpenApi->getStreamingApiOrderbookUnsubscribeJson("BBG000BN56Q9", 20) );

    showJsonWithTitle( "Candle subscribe"     , pOpenApi->getStreamingApiCandleSubscribeJson     ("BBG000BN56Q9", "5MIN") );
    showJsonWithTitle( "Candle unsubscribe"   , pOpenApi->getStreamingApiCandleUnsubscribeJson   ("BBG000BN56Q9", "5MIN") );
    
    showJsonWithTitle( "InstrumentInfo subscribe"   , pOpenApi->getStreamingApiInstrumentInfoSubscribeJson     ("BBG000BN56Q9") );
    showJsonWithTitle( "InstrumentInfo unsubscribe" , pOpenApi->getStreamingApiInstrumentInfoUnsubscribeJson   ("BBG000BN56Q9") );

    return 0;
}



