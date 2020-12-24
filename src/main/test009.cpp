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
    QCoreApplication::setApplicationName("test009");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;


    //------------------------------
    cout<<"Launched from : "<<QDir::currentPath().toStdString()<<endl;
    cout<<"Launched exe  : "<<QCoreApplication::applicationFilePath().toStdString()<<endl;
    cout<<"Launched from : "<<QCoreApplication::applicationDirPath().toStdString()<<endl;

    //------------------------------
    namespace tkf=invest_openapi;
    using tkf::config_helpers::lookupForConfigFile;
    using tkf::config_helpers::FileReadable;


    //------------------------------
    QSharedPointer<tkf::IOpenApi> pOpenApi = tkf::createOpenApi( lookupForConfigFile( "config.properties", "conf;config", FileReadable() )
                                                               , lookupForConfigFile( "auth.properties"  , "conf;config", FileReadable() )
                                                               );

    auto instrumentList = tkf::toInstrumentList<double>( tkf::joinAndGetPayload(pOpenApi->marketInstruments()).getInstruments() );

    auto isinFigiMap   = tkf::makeIsinFigiMap(instrumentList);
    auto tickerFigiMap = tkf::makeTickerFigiMap(instrumentList);

    
    //------------------------------
    //tkf::ISanboxOpenApi* pSandboxOpenApi = dynamic_cast<tkf::ISanboxOpenApi*>(pOpenApi.get());
    tkf::ISanboxOpenApi* pSandboxOpenApi = tkf::getSandboxApi(pOpenApi);

    if (pSandboxOpenApi)
    {
        //------------------------------
        auto sandboxRegisterRes = tkf::joinAndGetPayload( pSandboxOpenApi->sandboxRegister(tkf::BrokerAccountType::eBrokerAccountType::TINKOFF) ); // TINKOFFIIS
        pSandboxOpenApi->setBrokerAccountId( sandboxRegisterRes.getBrokerAccountId() );

        //------------------------------
        tkf::CurrenciesConfig currenciesConfig;
        QSettings sandboxSettings( lookupForConfigFile( "sandbox.properties", "conf;config", FileReadable() ), QSettings::IniFormat);

        currenciesConfig = tkf::CurrenciesConfig( sandboxSettings );
        auto currenciesList = currenciesConfig.getCurrencyConfigs("RUB;USD;EUR;GBP;HKD;CHF;JPY;CNY;TRY");

        auto res = tkf::joinAndGetPayload(pSandboxOpenApi->sandboxCurrenciesBalanceSet( currenciesList ));

        //------------------------------
        auto sandboxPositionsBalance = tkf::readSandboxPositionsConfig( sandboxSettings, true  /* readStrict */ );
        if (!sandboxPositionsBalance.empty())
        {
            sandboxPositionsBalance = tkf::positionsConfigToFigi( sandboxPositionsBalance, isinFigiMap, tickerFigiMap );
            res = tkf::joinAndGetPayload( pSandboxOpenApi->sandboxPositionsBalanceSet(sandboxPositionsBalance) );
        }

    }

    //------------------------------
    QDateTime nowDateTime     = QDateTime::currentDateTime();
    QDateTime hourAgoDateTime = nowDateTime.addSecs(-3600);

    auto candles = tkf::joinAndGetPayload( pOpenApi->marketCandles( tickerFigiMap["ROSN"], hourAgoDateTime, nowDateTime, "1min") );

    //------------------------------
    auto orderbook = tkf::joinAndGetPayload( pOpenApi->marketOrderbook( tickerFigiMap["ROSN"] ) );

    //------------------------------
    auto userAccounts = tkf::joinAndGetPayload( pOpenApi->userAccounts() );
    
    //------------------------------
    auto portfolioCurrencies = tkf::joinAndGetPayload( pOpenApi->portfolioCurrencies() );
    
    //------------------------------
    auto portfolio = tkf::joinAndGetPayload(pOpenApi->portfolio());

    //------------------------------
    qDebug() << "------------------------------";

    auto orders = tkf::joinAndGetPayload(pOpenApi->orders());
    qDebug() << "Orders (before): " << orders; //.asJson();

    auto 
    marketOrder = tkf::joinAndGetPayload(pOpenApi->ordersMarketOrder( tickerFigiMap["ROSN"], "BUY", 1 ) );
    qDebug() << "Market order: " << marketOrder;
    orders = tkf::joinAndGetPayload(pOpenApi->orders());
    qDebug() << "Orders (after buy): " << orders; //.asJson();

    marketOrder = tkf::joinAndGetPayload(pOpenApi->ordersMarketOrder( tickerFigiMap["ROSN"], "SELL", 1 ) );
    qDebug() << "Market order: " << marketOrder;
    orders = tkf::joinAndGetPayload(pOpenApi->orders());
    qDebug() << "Orders (after sell): " << orders; //.asJson();

    //------------------------------
    qDebug() << "------------------------------";

    auto 
    limitOrder = tkf::joinAndGetPayload(pOpenApi->ordersLimitOrder( tickerFigiMap["ROSN"], "BUY", 1, 420.0 ) );
    qDebug() << "Limit order: " << marketOrder;
    orders = tkf::joinAndGetPayload(pOpenApi->orders());
    qDebug() << "Orders (after buy): " << orders; //.asJson();

    limitOrder = tkf::joinAndGetPayload(pOpenApi->ordersLimitOrder( tickerFigiMap["ROSN"], "SELL", 1, 440.0 ) );
    qDebug() << "Limit order: " << marketOrder;
    orders = tkf::joinAndGetPayload(pOpenApi->orders());
    qDebug() << "Orders (after sell): " << orders; //.asJson();
    
    //------------------------------
    qDebug() << "------------------------------";

    //QDateTime nowDateTime     = QDateTime::currentDateTime();
    QDateTime yearAgoDateTime = nowDateTime.addYears(-1);

    auto 
    operations = tkf::joinAndGetPayload(pOpenApi->operations( nowDateTime, yearAgoDateTime, tickerFigiMap["ROSN"] ) );

    //------------------------------

    if (pSandboxOpenApi)
    {
        auto 
        res = tkf::joinAndGetPayload( pSandboxOpenApi->sandboxClear() );
        res = tkf::joinAndGetPayload( pSandboxOpenApi->sandboxRemove() );
    }

    
    return 0;
}



