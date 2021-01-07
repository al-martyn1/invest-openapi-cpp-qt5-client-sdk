/*! \file
    \brief New style API test

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
#include "invest_openapi/positions_config.h"

#include "invest_openapi/invest_openapi.h"

#include "invest_openapi/utility.h"


inline
void printQStringList( QString indent, QStringList strings )
{
    using std::cout;
    using std::endl;

    for (int i = 0; i < strings.size(); ++i)
    {
        QString s = strings.at(i).trimmed();
        cout << indent.toStdString() << s.toStdString() << endl;
    }

    cout << endl;

}


INVEST_OPENAPI_MAIN()
{
    //------------------------------
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("test008");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;


    //------------------------------
    cout<<"Launched from : "<<QDir::currentPath().toStdString()<<endl;
    cout<<"Launched exe  : "<<QCoreApplication::applicationFilePath().toStdString()<<endl;
    cout<<"Path to exe   : "<<QCoreApplication::applicationDirPath().toStdString()<<endl;

    //------------------------------
    namespace tkf=invest_openapi;
    using tkf::config_helpers::lookupForConfigFile;
    using tkf::config_helpers::FileReadable;


    //------------------------------
    QSharedPointer<tkf::IOpenApi> pOpenApi = tkf::createOpenApi( lookupForConfigFile( "config.properties"  , "conf;config", FileReadable() )
                                                               , lookupForConfigFile( "auth.properties"    , "conf;config", FileReadable() )
                                                               , lookupForConfigFile( "logging.properties" , "conf;config", FileReadable() )
                                                               );

    auto marketInstruments = pOpenApi->marketInstruments();
    marketInstruments->join();
    auto marketInstrumentList = marketInstruments->value.getPayload().getInstruments();
    auto instrumentList = tkf::toInstrumentList<double>(marketInstrumentList);

    auto isinFigiMap   = tkf::makeIsinFigiMap(instrumentList);
    auto tickerFigiMap = tkf::makeTickerFigiMap(instrumentList);

    
    //------------------------------
    //tkf::ISanboxOpenApi* pSandboxOpenApi = dynamic_cast<tkf::ISanboxOpenApi*>(pOpenApi.get());
    tkf::ISanboxOpenApi* pSandboxOpenApi = tkf::getSandboxApi(pOpenApi);

    if (pSandboxOpenApi)
    {
        //------------------------------
        auto sandboxRegisterRes = pSandboxOpenApi->sandboxRegister(tkf::BrokerAccountType::eBrokerAccountType::TINKOFF); // TINKOFFIIS
        sandboxRegisterRes->join();
        tkf::checkAbort(sandboxRegisterRes);

        pSandboxOpenApi->setBrokerAccountId( sandboxRegisterRes->value.getPayload().getBrokerAccountId() );

        {
            auto res = pSandboxOpenApi->sandboxCurrenciesBalanceSet( tkf::SandboxCurrency::eSandboxCurrency::RUB, 1000.0 );
            res->join();
            tkf::checkAbort(res);
        }

        //------------------------------
        tkf::CurrenciesConfig currenciesConfig;
        QSettings sandboxSettings( lookupForConfigFile( "sandbox.properties", "conf;config", FileReadable() ), QSettings::IniFormat);

        currenciesConfig = tkf::CurrenciesConfig( sandboxSettings );
        auto currenciesList = currenciesConfig.getCurrencyConfigs("RUB;USD;EUR;GBP;HKD;CHF;JPY;CNY;TRY");

        auto res = pSandboxOpenApi->sandboxCurrenciesBalanceSet( currenciesList );
        res->join();
        auto resPtr = res.get();
        //tkf::dumpIfError( setCurrenciesListResult );
        tkf::checkAbort(res);

        //------------------------------
        auto sandboxPositionsBalance = tkf::readSandboxPositionsConfig( sandboxSettings, true  /* readStrict */ );
        if (!sandboxPositionsBalance.empty())
        {
            sandboxPositionsBalance = tkf::positionsConfigToFigi( sandboxPositionsBalance, isinFigiMap, tickerFigiMap );
            res = pSandboxOpenApi->sandboxPositionsBalanceSet(sandboxPositionsBalance);
            res->join();
            tkf::checkAbort(res);
        }

    }

    //------------------------------
    QDateTime nowDateTime     = QDateTime::currentDateTime();
    QDateTime hourAgoDateTime = nowDateTime.addSecs(-3600);

    auto candlesResponse = pOpenApi->marketCandles( tickerFigiMap["ROSN"], hourAgoDateTime, nowDateTime, "1min");
    candlesResponse->join();
    tkf::checkAbort(candlesResponse);
    auto candles = candlesResponse->value.getPayload();

    //------------------------------
    auto orderbookResponse = pOpenApi->marketOrderbook( tickerFigiMap["ROSN"] );
    orderbookResponse->join();
    tkf::checkAbort(orderbookResponse);
    //QList<Order> 
    auto orderbook = orderbookResponse->value.getPayload();

    //------------------------------
    auto userAccountsResponse = pOpenApi->userAccounts();
    userAccountsResponse->join();
    tkf::checkAbort(userAccountsResponse);
    auto userAccounts = userAccountsResponse->value.getPayload();
    
    //------------------------------
    auto portfolioCurrenciesResponse = pOpenApi->portfolioCurrencies();
    portfolioCurrenciesResponse->join();
    tkf::checkAbort(portfolioCurrenciesResponse);
    auto portfolioCurrencies = portfolioCurrenciesResponse->value.getPayload();
    
    //------------------------------
    auto portfolioResponse = pOpenApi->portfolio();
    portfolioResponse->join();
    tkf::checkAbort(portfolioResponse);
    auto portfolio = portfolioResponse->value.getPayload();

    auto portfolio2 = tkf::joinAndGetPayload(pOpenApi->portfolio());
    
    //------------------------------
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



