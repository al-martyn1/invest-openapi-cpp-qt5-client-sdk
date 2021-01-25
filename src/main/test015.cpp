/*! \file
    \brief Generator schemas shower in predefined order

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
#include "invest_openapi/model_to_strings.h"


INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("test015");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;


    cout<<"Launched exe  : "<<QCoreApplication::applicationFilePath().toStdString()<<endl;

    cout << endl;

    namespace tkf=invest_openapi;
    using tkf::config_helpers::lookupForConfigFile;
    using tkf::config_helpers::FileReadable;

    QSet<QString>         allSqlTables  = tkf::modelMakeAllSqlTablesSet_SQLITE();
    QMap<QString,QString> allSqlSchemas = tkf::modelMakeAllSqlShemas_SQLITE();

    QVector<QString> refBooks;
    refBooks.append("ORDER_TYPE");
    refBooks.append("ORDER_STATUS");
    refBooks.append("OPERATION_TYPE");
    refBooks.append("CANDLE_RESOLUTION");
    refBooks.append("CURRENCY");
    refBooks.append("INSTRUMENT_TYPE");
    //refBooks.append("BROKER_ACCOUNT_TYPE");
    refBooks.append("MARKET_INSTRUMENT");
    //refBooks.append("");

    /*
        MARKET_INSTRUMENT
        - SEARCH_MARKET_INSTRUMENT
        - CURRENCY_POSITION
        - SANDBOX_ACCOUNT
        - USER_ACCOUNT
        - MONEY_AMOUNT
        ? CANDLE
        ? PLACED_LIMIT_ORDER
        - PORTFOLIO_POSITION
        ? PLACED_MARKET_ORDER
        ? OPERATION_TRADE
        ? ORDER

        // Non-generated
        modelMakeSqlSchemaStringVector_SQLITE<BrokerAccountType>
        modelMakeSqlSchemaStringVector_SQLITE<Currency>
        modelMakeSqlSchemaStringVector_SQLITE<InstrumentType>
        modelMakeSqlSchemaStringVector_SQLITE<CandleResolution>
        modelMakeSqlSchemaStringVector_SQLITE<OperationType>
        modelMakeSqlSchemaStringVector_SQLITE<OrderStatus>
        modelMakeSqlSchemaStringVector_SQLITE<OrderType>

     */

    for( auto sqlTable : allSqlTables )
    {
        auto it = allSqlSchemas.find(sqlTable);
        if (it==allSqlSchemas.end())
        {
            cout << "Table '" << sqlTable.toStdString() << "' not found in schemes" << endl << endl;
        }
        else
        {
            cout << "Table '" << sqlTable.toStdString() << "' schema:" << endl;
            cout << it->toStdString() << endl << endl;
            // QString key = *it;
            // cout << "Table '" << sqlTable.toStdString() << "' schema:" << endl;
            // //cout << it->second << endl << endl;
            // cout << allSqlSchemas[key].toStdString() << endl << endl;
        }
    }


    

    
    return 0;
}



