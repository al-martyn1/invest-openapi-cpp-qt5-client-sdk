/*! \file
    \brief Sending sandbox register request test.

 */

#include <iostream>

#include <QCoreApplication>
#include <QString>
#include <QSettings>
#include <QTest>


#include "invest_openapi/api_config.h"
#include "invest_openapi/auth_config.h"

#include "invest_openapi/invest_openapi.h"
#include "invest_openapi/factory.h"
#include "invest_openapi/network_completable_future.h"

//NOTE: OpenSSL need to be installed



int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("test002");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;

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

        tkf::NetworkCompletableFuture< tkf::SandboxRegisterResponse >  sandboxRegisterResponse;
        sandboxRegisterResponse.connectTo( pSandboxApi, &tkf::SandboxApi::sandboxRegisterPostSignal, &tkf::SandboxApi::sandboxRegisterPostSignalE );

        /*
        QObject::connect( pSandboxApi, &tkf::SandboxApi::sandboxRegisterPostSignal
                        , &sandboxRegisterResponse
                        //, &tkf::NetworkCompletableFuture< tkf::SandboxRegisterResponse >::onComplete
                        , &tkf::NetworkCompletableFutureBase::onComplete
                        );
        */
        tkf::BrokerAccountType        brokerAccountType;
        brokerAccountType.setValue( tkf::BrokerAccountType::eBrokerAccountType::TINKOFF ); // TINKOFFIIS

        tkf::SandboxRegisterRequest   sandboxRegisterRequest;
        sandboxRegisterRequest.setBrokerAccountType(brokerAccountType);

        qDebug() << QDateTime::currentDateTime() << "Query SandboxRegister";
        pSandboxApi->sandboxRegisterPost(sandboxRegisterRequest);

        sandboxRegisterResponse.join();

    }

    unsigned counter = 0;
    while(++counter < 10000)
    {
        QTest::qWait(1);
    }
   
    return 0;
}