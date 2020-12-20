/*! \file
    \brief Read configuration files test
 */

#include <iostream>

#include <QCoreApplication>
#include <QString>
#include <QSettings>

#include "invest_openapi/api_config.h"
#include "invest_openapi/auth_config.h"




int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("test001");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;

    namespace tkf=invest_openapi;

    tkf::ApiConfig  apiConfig  = tkf::ApiConfig("./config.properties");
    tkf::AuthConfig authConfig = tkf::AuthConfig("./auth.properties");

    cout << "URL        : {" << apiConfig.url.toStdString() << "}" << endl;
    cout << "Sandbox URL: {" << apiConfig.urlSandbox.toStdString() << "}" << endl;
    cout << "SandboxMode: {" << apiConfig.urlStreaming.toStdString() << "}" << endl;

    cout << "Token      : {" << authConfig.token.toStdString() << "}" << endl;
    cout << "SandboxMode: {" << authConfig.sandboxMode << "}" << endl;
   
    return 0;
}