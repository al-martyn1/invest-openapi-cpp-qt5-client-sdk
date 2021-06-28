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

#include "invest_openapi/db_utils.h"
#include "invest_openapi/ioa_utils.h"
#include "invest_openapi/ioa_ostream.h"



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
    qDebug().nospace().noquote() << "" ;

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
    portolioRes = pOpenApi->portfolio();

    portolioRes->join();
    tkf::checkAbort(portolioRes);

    auto portfolio = portolioRes->value.getPayload();
    auto /* QList<PortfolioPosition> */ portfolioPositions = portfolio.getPositions();

    for( auto pos : portfolioPositions )
    {
        #if 0

            // Old version

            qDebug().nospace().noquote() << "";
           
            marty::Decimal balance = pos.getBalance();
            marty::Decimal blocked = pos.getBlocked();
            std::string balanceStr = balance.toString();
            if (blocked!=0)
            {
                balanceStr += std::string(" ( ") + blocked.toString() + std::string(" blocked)");
            }
           
            cout << pos.getFigi() << " - " << balanceStr                  << ", " 
                 << "price: " << pos.getAveragePositionPrice()            << ", " 
                 // << "price NoNkd: " << pos.getAveragePositionPriceNoNkd() << ", " 
                 << pos.getInstrumentType() 
                 << ", " 
                 << pos.getTicker() 
                 << " - " << pos.getName()
                 << endl
                 ;

        #else

            // New version, more detailed and accurate

            cout << pos.getTicker() << "/" << pos.getFigi() << ", "
                 << "Balance: " << pos.getBalance() << ", "
                 << "Blocked: " << pos.getBlocked() << " - "
                 << "AvgPrice: " << pos.getAveragePositionPrice() << ", "
                 // << "AvgPriceNoNkd: " << pos.getAveragePositionPriceNoNkd() << " - " // Хз чо, валидных значений не видел в своём портфеле
                 << "ExpectedYeld: " << pos.getExpectedYield() << " - "
                 << "Lots: " << pos.getLots() << " - " // Дублирует Balance с учётом размера лота
                 << pos.getInstrumentType() << " - "
                 << pos.getName()
                 << endl;

        #endif
    }


    /*
    [X] QString getFigi() const;
    [X] QString getTicker() const;
    [X] InstrumentType getInstrumentType() const;
    [X] marty::Decimal getBalance() const;
    [X] marty::Decimal getBlocked() const;
    [X] MoneyAmount getExpectedYield() const;
    [X] qint32 getLots() const;
    [X] MoneyAmount getAveragePositionPrice() const;
    [X] MoneyAmount getAveragePositionPriceNoNkd() const;
    [X] QString getName() const;


    */

    return 0;



}



