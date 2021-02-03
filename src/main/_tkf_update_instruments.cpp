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

    QStringList configSubfolders = QString("conf;config").split( ';', Qt::SkipEmptyParts );

    // QSettings configProperties ( tkf::config_helpers::lookupForConfigFile( "config.properties" , configSubfolders, FileReadable() ), QSettings::IniFormat);
    // QSettings authProperties   ( tkf::config_helpers::lookupForConfigFile( "auth.properties"   , configSubfolders, FileReadable() ), QSettings::IniFormat);
    // QSettings sandboxProperties( tkf::config_helpers::lookupForConfigFile( "sandbox.properties", configSubfolders, FileReadable() ), QSettings::IniFormat);

    auto apiConfig     = tkf::ApiConfig    ( tkf::config_helpers::lookupForConfigFile( "config.properties" , configSubfolders, FileReadable() ) );
    auto authConfig    = tkf::AuthConfig   ( tkf::config_helpers::lookupForConfigFile( "auth.properties"   , configSubfolders, FileReadable() ) );
    auto loggingConfig = tkf::LoggingConfig( tkf::config_helpers::lookupForConfigFile( "logging.properties", configSubfolders, FileReadable() ) );

    QSharedPointer<tkf::IOpenApi> pOpenApi = tkf::createOpenApi( apiConfig, authConfig, loggingConfig );

    tkf::ISanboxOpenApi* pSandboxOpenApi = tkf::getSandboxApi(pOpenApi);

    if (pSandboxOpenApi)
    {
        //------------------------------
        auto sandboxRegisterRes = pSandboxOpenApi->sandboxRegister(tkf::BrokerAccountType::eBrokerAccountType::TINKOFF); // TINKOFFIIS
        sandboxRegisterRes->join();
        tkf::checkAbort(sandboxRegisterRes);

        pSandboxOpenApi->setBrokerAccountId( sandboxRegisterRes->value.getPayload().getBrokerAccountId() );
    }

    //auto instrumentList = tkf::toInstrumentList<double>( tkf::joinAndGetPayload(pOpenApi->marketInstruments()).getInstruments() );

    //auto isinFigiMap   = tkf::makeIsinFigiMap(instrumentList);
    //auto tickerFigiMap = tkf::makeTickerFigiMap(instrumentList);

    /*
    auto stocks     = pOpenApi->marketInstruments( tkf::InstrumentType("STOCK"   ) );
    auto currencies = pOpenApi->marketInstruments( tkf::InstrumentType("CURRENCY") );
    auto bonds      = pOpenApi->marketInstruments( tkf::InstrumentType("BOND"    ) );
    auto etfs       = pOpenApi->marketInstruments( tkf::InstrumentType("ETF"     ) );

    //response->join();
    //checkAbort(response);
    //return response->value.getPayload();

    */

    QVector< QSharedPointer< tkf::OpenApiCompletableFuture< tkf::MarketInstrumentListResponse > > > instrumentResults;
    instrumentResults.push_back( pOpenApi->marketInstruments( tkf::InstrumentType("STOCK"   ) ) );
    instrumentResults.push_back( pOpenApi->marketInstruments( tkf::InstrumentType("CURRENCY") ) );
    instrumentResults.push_back( pOpenApi->marketInstruments( tkf::InstrumentType("BOND"    ) ) );
    instrumentResults.push_back( pOpenApi->marketInstruments( tkf::InstrumentType("ETF"     ) ) );

    tkf::joinOpenApiCompletableFutures(instrumentResults);

    for( auto response : instrumentResults )
    {
        auto marketInstrumentList = response->value.getPayload();
        auto instrumentsList = marketInstrumentList.getInstruments();

        for(const auto &instrument : instrumentsList)
        {
        
        }
    }

    /*
    for(const auto &instrument : list)
    {
        auto key = instrument.isin.toUpper();
        if (key.isEmpty())
            continue;
        resMap[key] = instrument.figi;
    }

     */

    if (pSandboxOpenApi)
    {
        auto 
        res = pSandboxOpenApi->sandboxClear();
        res->join();
        tkf::checkAbort(res);

        res = pSandboxOpenApi->sandboxRemove();
        res->join();
        tkf::checkAbort(res);
    }



    return 0;
}



