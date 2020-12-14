#include <iostream>

#include <QCoreApplication>
#include <QString>
#include <QSettings>

#include "MarketInstrumentList.h"


int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("example001");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;

    QSettings settings("./config.properties", QSettings::IniFormat);

    QString token    = settings.value("token").toString();
    bool sandboxMode = settings.value("sandboxMode").toBool();

    cout << "Token      : {" << token.toStdString() << "}" << endl;
    cout << "SandboxMode: {" << sandboxMode << "}" << endl;
   
    return 0;
}