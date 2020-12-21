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


#include "invest_openapi/invest_openapi.h"



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
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("test008");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;


    cout<<"Launched from : "<<QDir::currentPath().toStdString()<<endl;
    cout<<"Launched exe  : "<<QCoreApplication::applicationFilePath().toStdString()<<endl;
    cout<<"Launched from : "<<QCoreApplication::applicationDirPath().toStdString()<<endl;

    namespace tkf=invest_openapi;
    using tkf::config_helpers::lookupForConfigFile;
    using tkf::config_helpers::FileReadable;


    QSharedPointer<tkf::IOpenApi> pOpenApi = tkf::createOpenApi( lookupForConfigFile( "config.properties", "conf;config", FileReadable() )
                                                          , lookupForConfigFile( "auth.properties"  , "conf;config", FileReadable() )
                                                          );
    
    tkf::ISanboxOpenApi* pSandboxOpenApi = dynamic_cast<tkf::ISanboxOpenApi*>(pOpenApi.get());

    if (pSandboxOpenApi)
    {
        auto sandboxRegisterRes = pSandboxOpenApi->sandboxRegister(tkf::BrokerAccountType::eBrokerAccountType::TINKOFF); // TINKOFFIIS
        sandboxRegisterRes->join();

        cout << "sandboxRegister -----------------" << endl << sandboxRegisterRes->value.asJson().toStdString() << endl;

        pSandboxOpenApi->setBrokerAccountId( sandboxRegisterRes->value.getPayload().getBrokerAccountId() );

        {
            auto res = pSandboxOpenApi->sandboxCurrenciesBalanceSet( tkf::SandboxCurrency::eSandboxCurrency::RUB, 1000.0 );
            res->join();
            cout << "sandboxCurrenciesBalanceSet -----------------" << endl << res->value.asJson().toStdString() << endl;
        }

        tkf::CurrenciesConfig currenciesConfig;
        try
        {
            currenciesConfig = tkf::CurrenciesConfig( lookupForConfigFile( "sandbox.properties", "conf;config", FileReadable() ) );
            auto curConf = currenciesConfig.getCurrenciesConfigs("RUB;USD;EUR;GBP;HKD;CHF;JPY;CNY;TRY");

        }
        catch(...)
        {}
        
         //= tkf::CurrencyConfig
    }

    //pSandboxOpenApi->
    
    return 0;
}



