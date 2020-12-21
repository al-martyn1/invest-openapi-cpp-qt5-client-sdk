/*! \file
    \brief Sending sandbox register request test.
 */

#include <iostream>
#include <exception>
#include <stdexcept>

#include <QCoreApplication>
#include <QString>
#include <QSettings>
#include <QTest>
#include <QDir>

#define TEST_MODE

#include "invest_openapi/api_config.h"
#include "invest_openapi/auth_config.h"

#include "invest_openapi/invest_openapi.h"
#include "invest_openapi/factory.h"
#include "invest_openapi/openapi_completable_future.h"



//NOTE: OpenSSL need to be installed


//#define TRY_QT_CONCURENT


//int main(int argc, char* argv[])
INVEST_OPENAPI_MAIN()
{


    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("test002");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;


    cout<<"Launched from: "<<QDir::currentPath().toStdString()<<endl;


    namespace tkf=invest_openapi;

    tkf::ApiConfig  apiConfig  = tkf::ApiConfig("./config.properties");
    tkf::AuthConfig authConfig = tkf::AuthConfig("./auth.properties");

    tkf::OpenApiFactory factory = tkf::OpenApiFactory(apiConfig,authConfig);

    tkf::SandboxApi* pSandboxApi = factory.getSandboxApi( /* timeOut */ );
    tkf::MarketApi*  pMarketApi  = factory.getMarketApi( /* timeOut */ );

    if (pSandboxApi)
    {
        // https://wiki.qt.io/New_Signal_Slot_Syntax/ru

         QObject::connect( pSandboxApi, &tkf::SandboxApi::sandboxRegisterPostSignalE,
               []( tkf::SandboxRegisterResponse summary, QNetworkReply::NetworkError error_type, QString error_str )
               {
                   // cout<<error_str.toStdString()<<endl;
                   qDebug() << QDateTime::currentDateTime() << "Error: " << error_type << ":" << error_str;
               }
               );

         QObject::connect( pSandboxApi, &tkf::SandboxApi::sandboxRegisterPostSignalEFull,
               []( tkf::HttpRequestWorker *worker, QNetworkReply::NetworkError error_type, QString error_str )
               {
                   //cout<<error_str.toStdString()<<endl;
			       //QByteArray responseBody = worker->response;
               }
               );

         QObject::connect( pSandboxApi, &tkf::SandboxApi::sandboxRegisterPostSignal,
               []( tkf::SandboxRegisterResponse summary )
               {
                   // cout<<error_str.toStdString()<<endl;
                   qDebug() << QDateTime::currentDateTime() << "RegisterResponse: " << summary.asJson();
               }
               );

        tkf::OpenApiCompletableFuture< tkf::SandboxRegisterResponse >  sandboxRegisterResponse;

        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( &sandboxRegisterResponse, pSandboxApi, sandboxRegister, Post );
        //sandboxRegisterResponse.connectTo( pSandboxApi, &tkf::SandboxApi::sandboxRegisterPostSignal, &tkf::SandboxApi::sandboxRegisterPostSignalE );


        tkf::BrokerAccountType        brokerAccountType;
        brokerAccountType.setValue( tkf::BrokerAccountType::eBrokerAccountType::TINKOFF ); // TINKOFFIIS

        tkf::SandboxRegisterRequest   sandboxRegisterRequest;
        sandboxRegisterRequest.setBrokerAccountType(brokerAccountType);

        qDebug() << QDateTime::currentDateTime() << "Query SandboxRegister";

        #if defined(TRY_QT_CONCURENT)
            //QFuture<void> future = 
            QtConcurrent::run([=]() {
                                        // Code in this block will run in another thread
                                        pSandboxApi->sandboxRegisterPost(sandboxRegisterRequest);
                                    });
        #else
            pSandboxApi->sandboxRegisterPost(sandboxRegisterRequest);
        #endif

        qDebug() << QDateTime::currentDateTime() << "Query SandboxRegister finished, start waiting result";

        sandboxRegisterResponse.join();

        qDebug() << QDateTime::currentDateTime() << "Query SandboxRegister - got result";

    }

    /*
    unsigned counter = 0;
    while(++counter < 10000)
    {
        QTest::qWait(1);
    }
    */
   
    return 0;
}