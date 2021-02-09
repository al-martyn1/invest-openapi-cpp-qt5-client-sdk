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
    QCoreApplication::setApplicationName("_tkf_portfolio");
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

    auto // PortfolioResponse
    portolioRes = pOpenApi->portfolioCurrencies();

    portolioRes->join();
    tkf::checkAbort(portolioRes);

    auto portfolioCurrencies = portolioRes->value.getPayload();
    QList<tkf::CurrencyPosition> currencyList = portfolioCurrencies.getCurrencies();

    for( auto currencyPos : currencyList )
    {
        qDebug().nospace().noquote() << "";

        marty::Decimal balance = currencyPos.getBalance();
        marty::Decimal blocked = currencyPos.getBlocked();
        std::string balanceStr = balance.toString();
        if (blocked!=0)
        {
            balanceStr += std::string(" ( ") + blocked.toString() + std::string(" blocked)");
        }

        qDebug().nospace().noquote() << currencyPos.getCurrency().asJson() << " : " << QString::fromStdString(balanceStr);
    }


    /*
    qint32 getLots() const;
    void setLots(const qint32 &lots);

    MoneyAmount getAveragePositionPrice() const;
    void setAveragePositionPrice(const MoneyAmount &average_position_price);

    MoneyAmount getAveragePositionPriceNoNkd() const;
    void setAveragePositionPriceNoNkd(const MoneyAmount &average_position_price_no_nkd);

    QString getName() const;
    void setName(const QString &name);
    */

    return 0;



}



