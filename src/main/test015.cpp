/*! \file
    \brief Generator schemas shower first refBooks

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

    QSet<QString>         usedTables;
    QSet<QString>         unusedTables;

    QVector<QString> refBooks;
    
    refBooks.append("BROKER_ACCOUNT_TYPE");
    refBooks.append("CURRENCY");
    refBooks.append("INSTRUMENT_TYPE");
    refBooks.append("CANDLE_RESOLUTION");
    refBooks.append("OPERATION_TYPE");
    refBooks.append("ORDER_STATUS");
    refBooks.append("ORDER_TYPE");
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

    allSqlSchemas[QString("BROKER_ACCOUNT_TYPE")] = tkf::modelMakeSqlCreateTableSchema_SQLITE( tkf::modelMakeSqlSchemaStringVector_SQLITE<tkf::BrokerAccountType>(QString(), false ) );
    allSqlSchemas[QString("CURRENCY"           )] = tkf::modelMakeSqlCreateTableSchema_SQLITE( tkf::modelMakeSqlSchemaStringVector_SQLITE<tkf::Currency         >(QString(), false ) );
    allSqlSchemas[QString("INSTRUMENT_TYPE"    )] = tkf::modelMakeSqlCreateTableSchema_SQLITE( tkf::modelMakeSqlSchemaStringVector_SQLITE<tkf::InstrumentType   >(QString(), false ) );
    allSqlSchemas[QString("CANDLE_RESOLUTION"  )] = tkf::modelMakeSqlCreateTableSchema_SQLITE( tkf::modelMakeSqlSchemaStringVector_SQLITE<tkf::CandleResolution >(QString(), false ) );
    allSqlSchemas[QString("OPERATION_TYPE"     )] = tkf::modelMakeSqlCreateTableSchema_SQLITE( tkf::modelMakeSqlSchemaStringVector_SQLITE<tkf::OperationType    >(QString(), false ) );
    allSqlSchemas[QString("ORDER_STATUS"       )] = tkf::modelMakeSqlCreateTableSchema_SQLITE( tkf::modelMakeSqlSchemaStringVector_SQLITE<tkf::OrderStatus      >(QString(), false ) );
    allSqlSchemas[QString("ORDER_TYPE"         )] = tkf::modelMakeSqlCreateTableSchema_SQLITE( tkf::modelMakeSqlSchemaStringVector_SQLITE<tkf::OrderType        >(QString(), false ) );
    // allSqlSchemas[""] = tkf::modelMakeSqlSchemaStringVector_SQLITE<>(QString(), false );

    for( auto sqlTable : refBooks )
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

            usedTables.insert(sqlTable);
        }
    }

    for( auto sqlTable : allSqlTables )
    {
        auto it = usedTables.find(sqlTable);
        if (it==usedTables.end())
        {
            // Not used
            unusedTables.insert(sqlTable);
        }
    }

    cout<<endl<<endl<<"!!! Unused tables"<<endl<<endl;

    for( auto sqlTable : unusedTables )
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
        }
    }

    return 0;
}



