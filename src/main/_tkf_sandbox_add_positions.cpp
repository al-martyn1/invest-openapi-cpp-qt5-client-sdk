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
    QCoreApplication::setApplicationName("_tkf_update_instruments");
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

    //NOTE: !!! sandbox-token must be set in 'auth.properties'
    authConfig.sandboxMode = true;


    QSharedPointer<tkf::IOpenApi> pOpenApi = tkf::createOpenApi( apiConfig, authConfig, loggingConfig );

    tkf::ISanboxOpenApi* pSandboxOpenApi = tkf::getSandboxApi(pOpenApi);

    if (pSandboxOpenApi)
    {
        pSandboxOpenApi->setBrokerAccountId( authConfig.getBrokerAccountId() );
    }

    QVector< QSharedPointer< tkf::OpenApiCompletableFuture< tkf::MarketInstrumentListResponse > > > instrumentResults;
    instrumentResults.push_back( pOpenApi->marketInstruments( tkf::InstrumentType("STOCK"   ) ) );
    instrumentResults.push_back( pOpenApi->marketInstruments( tkf::InstrumentType("CURRENCY") ) );
    instrumentResults.push_back( pOpenApi->marketInstruments( tkf::InstrumentType("BOND"    ) ) );
    instrumentResults.push_back( pOpenApi->marketInstruments( tkf::InstrumentType("ETF"     ) ) );

    QList<tkf::MarketInstrument> allInstrumentsList;

    for( auto rsp : instrumentResults )
    {
        auto marketInstrumentList                = rsp->value.getPayload();
        QList<tkf::MarketInstrument> instrumentsList  = marketInstrumentList.getInstruments();

        for( auto instrumentInfo : instrumentsList )
        {
            allInstrumentsList.push_back(instrumentInfo);
        }
    }

    //auto instrumentList = tkf::toInstrumentList<double>( tkf::joinAndGetPayload(pOpenApi->marketInstruments(tkf::InstrumentType("STOCK"))).getInstruments() );

    auto isinFigiMap   = tkf::makeIsinFigiMap  (allInstrumentsList);
    auto tickerFigiMap = tkf::makeTickerFigiMap(allInstrumentsList);


    return 0;
}



