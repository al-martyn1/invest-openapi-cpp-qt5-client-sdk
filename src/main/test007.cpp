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

#define TEST_MODE

#include "invest_openapi/config_helpers.h"
#include "invest_openapi/api_config.h"
#include "invest_openapi/auth_config.h"

#include "invest_openapi/invest_openapi.h"
#include "invest_openapi/factory.h"
#include "invest_openapi/openapi_completable_future.h"




INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("test007");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;


    cout<<"Launched from : "<<QDir::currentPath().toStdString()<<endl;
    cout<<"Launched exe  : "<<QCoreApplication::applicationFilePath().toStdString()<<endl;
    cout<<"Launched from : "<<QCoreApplication::applicationDirPath().toStdString()<<endl;

    using invest_openapi::config_helpers::lookupForConfigFile;
    using invest_openapi::config_helpers::FileReadable;

    namespace tkf=invest_openapi;


    tkf::OpenApiFactory factory = tkf::OpenApiFactory( lookupForConfigFile( "config.properties", "conf;config", FileReadable() )
                                                     , lookupForConfigFile( "auth.properties"  , "conf;config", FileReadable() )
                                                     );

    QSharedPointer< tkf::SandboxApi > pApi = factory.getApiImpl< tkf::SandboxApi >();

    if (pApi)
    {
        tkf::BrokerAccountType        brokerAccountType;
        brokerAccountType.setValue( tkf::BrokerAccountType::eBrokerAccountType::TINKOFF ); // TINKOFFIIS

        tkf::SandboxRegisterRequest   sandboxRegisterRequest;
        sandboxRegisterRequest.setBrokerAccountType(brokerAccountType);

        tkf::OpenApiCompletableFuture< tkf::SandboxRegisterResponse >  sandboxRegisterResponse;
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( &sandboxRegisterResponse, &(*pApi), sandboxRegister, Post );

        pApi->sandboxRegisterPost(sandboxRegisterRequest);

        sandboxRegisterResponse.join();

        cout << sandboxRegisterResponse.value.asJson().toStdString() << endl;
    }


    
    return 0;
}



