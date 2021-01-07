/*! \file
    \brief Calculate timings
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

#define TEST_MODE

#include "invest_openapi/api_config.h"
#include "invest_openapi/auth_config.h"

#include "invest_openapi/invest_openapi.h"
#include "invest_openapi/factory.h"
#include "invest_openapi/openapi_completable_future.h"



//NOTE: OpenSSL need to be installed

INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("test003");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;


    cout<<"Launched from : "<<QDir::currentPath().toStdString()<<endl;
    cout<<"Launched exe  : "<<QCoreApplication::applicationFilePath().toStdString()<<endl;
    cout<<"Path to exe   : "<<QCoreApplication::applicationDirPath().toStdString()<<endl;

    cout<<"Going up to the root"<<endl;

    QDir testDir = QDir(QCoreApplication::applicationDirPath());
    while(!testDir.isRoot())
    {
        cout<<"  " <<testDir.absolutePath().toStdString()<<endl;
        cout<<"    " <<testDir.filePath("test.txt").toStdString()<<endl;
        testDir.cdUp();
    }
    

    namespace tkf=invest_openapi;

    tkf::ApiConfig  apiConfig  = tkf::ApiConfig("./config.properties");
    tkf::AuthConfig authConfig = tkf::AuthConfig("./auth.properties");

    tkf::OpenApiFactory factory = tkf::OpenApiFactory(apiConfig,authConfig);

    tkf::SandboxApi* pSandboxApi = factory.getSandboxApi( /* timeOut */ );
    tkf::MarketApi*  pMarketApi  = factory.getMarketApi( /* timeOut */ );

    if (pSandboxApi)
    {

        qint64 sandboxRegisterPostIntervalSum     = 0;
        qint64 sandboxRegisterResponseIntervalSum = 0;
        qint64 totalSum                           = 0;

        qint64 i=0u;

        for(; i!=10; ++i)
        {
            tkf::OpenApiCompletableFuture< tkf::SandboxRegisterResponse >  sandboxRegisterResponse;
            INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( &sandboxRegisterResponse, pSandboxApi, sandboxRegister, Post );

            tkf::BrokerAccountType        brokerAccountType;
            brokerAccountType.setValue( tkf::BrokerAccountType::eBrokerAccountType::TINKOFF ); // TINKOFFIIS
           
            tkf::SandboxRegisterRequest   sandboxRegisterRequest;
            sandboxRegisterRequest.setBrokerAccountType(brokerAccountType);
            
            QElapsedTimer timer;
            timer.start();

            pSandboxApi->sandboxRegisterPost(sandboxRegisterRequest);

            auto sandboxRegisterPostInterval = timer.restart();

            sandboxRegisterResponse.join();

            auto sandboxRegisterResponseInterval = timer.restart();

            auto total = sandboxRegisterPostInterval + sandboxRegisterResponseInterval;

            qDebug() << "----------------------";
            qDebug() << "Performing POST took  " << sandboxRegisterPostInterval     << "milliseconds";
            qDebug() << "Performing join took  " << sandboxRegisterResponseInterval << "milliseconds";
            qDebug() << "Obtaining result took " << total << "milliseconds total";

            if (!i) continue; // skip first call - it initializes something

            sandboxRegisterPostIntervalSum     += sandboxRegisterPostInterval    ;
            sandboxRegisterResponseIntervalSum += sandboxRegisterResponseInterval;
            totalSum                           += total                          ;
        }

        qDebug() << "--------------------------------------------";
        qDebug() << "Performing POST took  " << (sandboxRegisterPostIntervalSum     / (i-1)) << "milliseconds average";
        qDebug() << "Performing join took  " << (sandboxRegisterResponseIntervalSum / (i-1)) << "milliseconds average";
        qDebug() << "Obtaining result took " << (totalSum / (i-1)) << "milliseconds total average";


    }

   
    return 0;
}