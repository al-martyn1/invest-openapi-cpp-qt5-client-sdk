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
#include <QString>
#include <QSettings>

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

#include "invest_openapi/marty_decimal.h"



INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("_tkf_sandbox_add_positions");
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
    auto sboxConfigFullFileName  = lookupForConfigFile( "sandbox.properties" , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );

    qDebug().nospace().noquote() << "Log  Config File: "<< logConfigFullFileName  ;
    qDebug().nospace().noquote() << "API  Config File: "<< apiConfigFullFileName  ;
    qDebug().nospace().noquote() << "Auth Config File: "<< authConfigFullFileName ;
    qDebug().nospace().noquote() << "SBox Config File: "<< sboxConfigFullFileName ;

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
    else
    {
        qDebug().nospace().noquote() << "Can't add positions while SandboxOpenApi is not available";
        return 0;
    }

    QVector< QSharedPointer< tkf::OpenApiCompletableFuture< tkf::MarketInstrumentListResponse > > > instrumentResults;
    instrumentResults.push_back( pOpenApi->marketInstruments( tkf::InstrumentType("STOCK"   ) ) );
    instrumentResults.push_back( pOpenApi->marketInstruments( tkf::InstrumentType("CURRENCY") ) );
    instrumentResults.push_back( pOpenApi->marketInstruments( tkf::InstrumentType("BOND"    ) ) );
    instrumentResults.push_back( pOpenApi->marketInstruments( tkf::InstrumentType("ETF"     ) ) );

    tkf::joinOpenApiCompletableFutures(instrumentResults);

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


    auto isinFigiMap   = tkf::makeIsinFigiMap  (allInstrumentsList);
    auto tickerFigiMap = tkf::makeTickerFigiMap(allInstrumentsList);
    auto figiTickerMap = tkf::makeFigiTickerMap(allInstrumentsList);

    // QJsonValue test

    double dbl = 1000.05;
    QJsonValue jv = QJsonValue(dbl);
    auto jvAsString = jv.toString();


    QSettings sandboxSettings(sboxConfigFullFileName, QSettings::IniFormat);


    //QStringList	sandboxSettingsAllKeys = sandboxSettings.allKeys();
    //qDebug().nospace().noquote() << "All keys in Sandbox Settings: " << sandboxSettingsAllKeys;

    qDebug().nospace().noquote() << "";

    QStringList currenciesList = sandboxSettings.value("sandbox.currencies").toStringList();
    qDebug().nospace().noquote() << "Set Sandbox Currencies: "<<currenciesList;

    for( auto currencyId : currenciesList )
    {
        qDebug().nospace().noquote() << "Set Sandbox currency balance for '"<< currencyId << "'";

        QString currencyPositionValueStr = sandboxSettings.value(QString("sandbox.currency.")+currencyId).toString();
        auto res = pSandboxOpenApi->sandboxCurrenciesBalanceSet( tkf::SandboxCurrency(currencyId)
                                                               , marty::Decimal::fromString( currencyPositionValueStr.toStdString() )
                                                               );

        res->join();
        tkf::checkAbort(res);
        qDebug().nospace().noquote() << "Set Sandbox currency balance for "<< currencyId << " - Done";
    }



    QStringList positionsList = sandboxSettings.value("sandbox.positions").toStringList();

    qDebug().nospace().noquote() << "Set Sandbox Positions: "<<positionsList;

    for( auto position : positionsList )
    {
        qDebug().nospace().noquote() << "Set Sandbox position balance for '"<< position << "'";

        QString figi;

        auto it = tickerFigiMap.find(position);
        if (it!=tickerFigiMap.end())
        {
            figi = it->second;
        }
        else if ( (it=isinFigiMap.find(position))!=isinFigiMap.end() )
        {
            figi = it->second;
        }
        else
        {
            figi = position; // FIGI itself taken
        }

        QString ticker;
        it = figiTickerMap.find(figi);
        if (it==figiTickerMap.end())
        {
            qDebug().nospace().noquote() << "Position '"<< position << "' - TICKER not found, may be wrong FIGI taken";
            continue;
        }

        ticker = it->second;

        qDebug().nospace().noquote() << "";

        qDebug().nospace().noquote() << "Set Sandbox position balance for position - FIGI: '"<<figi<<"', TICKER: '"<<ticker<<"'";

        //qDebug().nospace().noquote() << "positionValue: " << sandboxSettings.value(QString("sandbox.position.") + position);
        QString positionValueStr = sandboxSettings.value(QString("sandbox.position.") + position).toString();
        auto res = pSandboxOpenApi->sandboxPositionsBalanceSet( figi
                                                              , marty::Decimal::fromString( positionValueStr.toStdString() )
                                                              );
        res->join();
        tkf::checkAbort(res);

        qDebug().nospace().noquote() << "Set Sandbox position balance for '"<< position << "' - FIGI: '"<<figi<<"' , TICKER: '"<<ticker<<"' - Done";
        
    }


    return 0;
}



