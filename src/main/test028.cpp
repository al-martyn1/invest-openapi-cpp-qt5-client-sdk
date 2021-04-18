/*! \file
    \brief Configs lookup test

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


    QString jsonSample = "{"
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

    cout << endl;
    cout << "StreamingOrderbook test" << endl;
    cout << "FIGI  : " << streamingOrderbook.getFigi()  << endl;
    cout << "Depth : " << streamingOrderbook.getDepth() << endl;

    QList<tkf::OrderResponse> bids = streamingOrderbook.getBids();
    QList<tkf::OrderResponse> asks = streamingOrderbook.getAsks();

    cout << "Bids:" << endl;
    for( auto bid : bids )
    {
        cout << "    " << bid.getQuantity() << " by " << bid.getPrice() << endl;
    }

    cout << "Asks:" << endl;
    for( auto ask : asks )
    {
        cout << "    " << ask.getQuantity() << " by " << ask.getPrice() << endl;
    }


    
    return 0;
}



