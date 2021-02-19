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

#include "invest_openapi/ioa_ostream.h"


INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("test020");
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


    QDateTime curTime   = QDateTime::currentDateTime();
    QDateTime startDate = curTime.addYears(-1); //curTime.date().addYears(-1);

    // BBG004731354 - ROSN
    auto // OperationsResponse
    operationsRes = pOpenApi->operations( startDate, curTime, "BBG004731354" ); // Если не задать фигу, то ошибки нет, просто пустой список

    operationsRes->join();
    tkf::checkAbort(operationsRes);

    auto operations = operationsRes->value.getPayload();
    QList<tkf::Operation> opList = operations.getOperations();

    for( const auto &op : opList )
    {
        if (!op.isSet() || !op.isValid())
            continue;

        cout << "Operation ID      : " << op.getId()               << endl;
        cout << "Operation Status  : " << op.getStatus()           << endl;
        cout << "Figi              : " << op.getFigi()             << endl;
        cout << "Date & time       : " << op.getDate()             << endl;
        cout << "Commission        : " << op.getCommission()       << endl;
        cout << "Currency          : " << op.getCurrency()         << endl;
        cout << "Payment           : " << op.getPayment()          << endl;
        cout << "Price             : " << op.getPrice()            << endl;
        cout << "Quantity          : " << op.getQuantity()         << endl;
        cout << "Quantity Executed : " << op.getQuantityExecuted() << endl;
        cout << "Instrument Type   : " << op.getInstrumentType()   << endl;
        cout << "Nike called       : " << (op.isIsMarginCall()?"yes":"no") << endl;
        cout << "Operation Type    : " << op.getOperationType()    << endl;
        //cout << "        : " << op.get()   << endl;

        /*
        virtual bool isSet() const override;
        virtual bool isValid() const override;
       
        QString getId() const;                    // ID операции
        OperationStatus getStatus() const;        // Done, Decline, Progress
        QList<OperationTrade> getTrades() const;  // list
        MoneyAmount getCommission() const;
        Currency getCurrency() const;
        marty::Decimal getPayment() const;
        marty::Decimal getPrice() const;
        qint32 getQuantity() const;               // Число инструментов в выставленной заявке
        qint32 getQuantityExecuted() const;       // Число инструментов, исполненных в заявке
        QString getFigi() const;                  // Фига
        InstrumentType getInstrumentType() const; // Тип инструмента
        bool isIsMarginCall() const;              // Звонил ли Коля
        QDateTime getDate() const;
        OperationTypeWithCommission getOperationType() const;
        */

        cout << "------------------------------------------------" << endl;

    }

    
    #if 0
    auto // PortfolioResponse
    portolioRes = pOpenApi->portfolio();

    portolioRes->join();
    tkf::checkAbort(portolioRes);

    auto portfolio = portolioRes->value.getPayload();
    auto /* QList<PortfolioPosition> */ portfolioPositions = portfolio.getPositions();
    #endif



    // TKF_IOA_ABSTRACT_METHOD( OperationsResponse , operations ( const QDateTime &from, const QDateTime &to, const QString &figi, QString broker_account_id = QString()) );
    return 0;
}



