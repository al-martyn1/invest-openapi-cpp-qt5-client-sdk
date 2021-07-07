/*! \file
    \brief Configs lookup test

 */

#include <iostream>
#include <exception>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <map>
#include <set>

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
#include "invest_openapi/balance_config.h"

#include "invest_openapi/invest_openapi.h"
#include "invest_openapi/factory.h"
#include "invest_openapi/openapi_completable_future.h"

#include "invest_openapi/database_config.h"
#include "invest_openapi/database_manager.h"
#include "invest_openapi/qt_time_helpers.h"

#include "invest_openapi/db_utils.h"
#include "invest_openapi/ioa_utils.h"
#include "invest_openapi/ioa_ostream.h"



void printMapOfFigiSets( const std::map< QString, std::set<QString> > &m, const std::map< QString, QString > &figiToTicker )
{
    std::map< QString, std::set<QString> >::const_iterator it = m.begin();
    for(; it!=m.end(); ++it)
    {
        std::cout << it->first << " (" << invest_openapi::dictionaryGetValue(figiToTicker, it->first) << ") : " << invest_openapi::mergeString( it->second, QString(", ") ) << std::endl;
    }
}




INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("test022");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;
    //cout.precision(2);

    cout<<"Path to exe   : "<<QCoreApplication::applicationDirPath().toStdString()<<endl;

    cout << endl;

    namespace tkf=invest_openapi;
    using tkf::config_helpers::lookupForConfigFile;
    using tkf::config_helpers::FileReadable;

    QStringList lookupConfSubfolders = QString("conf;config").split( ';', Qt::SkipEmptyParts );


    auto logConfigFullFileName     = lookupForConfigFile( "logging.properties" , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto apiConfigFullFileName     = lookupForConfigFile( "config.properties"  , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto authConfigFullFileName    = lookupForConfigFile( "auth.properties"    , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto dbConfigFullFileName      = lookupForConfigFile( "database.properties", lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto balanceConfigFullFileName = lookupForConfigFile( "instruments.properties", lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto test022FullFileName       = lookupForConfigFile( "test022.properties" , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );

    qDebug().nospace().noquote() << "Log  Config File: "<< logConfigFullFileName  ;
    qDebug().nospace().noquote() << "API  Config File: "<< apiConfigFullFileName  ;
    qDebug().nospace().noquote() << "Auth Config File: "<< authConfigFullFileName ;
    qDebug().nospace().noquote() << "DB   Config     : "<< dbConfigFullFileName   ;
    qDebug().nospace().noquote() << "Instruments Config: "<< balanceConfigFullFileName;
    qDebug().nospace().noquote() << "Test022 Cfg File: "<< test022FullFileName    ;

    auto apiConfig     = tkf::ApiConfig    ( apiConfigFullFileName  );
    auto authConfig    = tkf::AuthConfig   ( authConfigFullFileName );
    auto balanceConfig = tkf::BalanceConfig( balanceConfigFullFileName );
    //auto balance_config.h

    bool slowMode = false;
    {
        QSettings settings(test022FullFileName, QSettings::IniFormat);
        slowMode = settings.value("slow-mode" ).toBool();
    }



    QSharedPointer<tkf::DatabaseConfig> pDatabaseConfig = QSharedPointer<tkf::DatabaseConfig>( new tkf::DatabaseConfig(dbConfigFullFileName, tkf::DatabasePlacementStrategyDefault()) );
    QSharedPointer<tkf::LoggingConfig>  pLoggingConfig  = QSharedPointer<tkf::LoggingConfig> ( new tkf::LoggingConfig(logConfigFullFileName) );
    auto loggingConfig = *pLoggingConfig;


    qDebug().nospace().noquote() << "Main DB name: " << pDatabaseConfig->dbMainFilename;

    QSharedPointer<QSqlDatabase> pMainSqlDb = QSharedPointer<QSqlDatabase>( new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE")) );
    pMainSqlDb->setDatabaseName( pDatabaseConfig->dbMainFilename );

    if (!pMainSqlDb->open())
    {
      qDebug() << pMainSqlDb->lastError().text();
      return 0;
    }

    QSharedPointer<tkf::IDatabaseManager> pMainDbMan = tkf::createMainDatabaseManager( pMainSqlDb, pDatabaseConfig, pLoggingConfig );

    pMainDbMan->applyDefDecimalFormatFromConfig( *pDatabaseConfig );



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


    std::map< QString, int > figiToId   = pMainDbMan->getDictionaryFromTable  ( "MARKET_INSTRUMENT", "FIGI,ID"  );
    std::map< QString, int > tickerToId = pMainDbMan->getDictionaryFromTable  ( "MARKET_INSTRUMENT", "TICKER,ID");
    std::map< QString, int > isinToId   = pMainDbMan->getDictionaryFromTable  ( "MARKET_INSTRUMENT", "ISIN,ID"  );
    std::map< int, QString > idToName   = pMainDbMan->getIdToFieldMapFromTable( "MARKET_INSTRUMENT", "ID,NAME"  );

    if (figiToId.empty())
    {
        qDebug().nospace().noquote() << "Something goes wrong. Possible DB is clean";
        return 1;
    }

    std::map< int, QString > idToFigi   = tkf::makeMapSwapKeyVal( figiToId );
    std::map< int, QString > idToTicker = tkf::makeMapSwapKeyVal( tickerToId );

    std::map< QString, QString > tickerToFigi = tkf::makeTransitionMap( tickerToId, idToFigi   );
    std::map< QString, QString > isinToFigi   = tkf::makeTransitionMap( isinToId  , idToFigi   );
    std::map< QString, QString > figiToTicker = tkf::makeTransitionMap( figiToId  , idToTicker );
    std::map< QString, QString > figiToName   = tkf::makeTransitionMap( figiToId  , idToName   );


    // Получаем портфолио - содержимое нашего инвест-портфельчика

    auto // PortfolioResponse
    portolioRes = pOpenApi->portfolio();

    portolioRes->join();
    tkf::checkAbort(portolioRes);

    auto portfolio = tkf::makeVectorFromList(portolioRes->value.getPayload().getPositions());

    std::map< QString, marty::Decimal   >                    portfolioFigiBalance;
    std::map< QString, std::map< QString, marty::Decimal > > portfolioFigiAveragePrice;
    std::map< QString, std::map< QString, marty::Decimal > > portfolioFigiExpectedYield;

    for( auto pos : portfolio )
    {
        auto posFigi = pos.getFigi().toUpper();
        portfolioFigiBalance[ posFigi ]      = pos.getBalance();

        tkf::MoneyAmount moneyAmount = pos.getAveragePositionPrice();
        QString moneyAmountCurrency  = moneyAmount.getCurrency().asJson().toUpper();

        portfolioFigiAveragePrice[posFigi][moneyAmountCurrency] = moneyAmount.getValue();


        auto expectedYield = pos.getExpectedYield();
        QString expectedYieldCurrency  = expectedYield.getCurrency().asJson().toUpper();

        portfolioFigiExpectedYield[posFigi][expectedYieldCurrency] = expectedYield.getValue();


        //portfolioFigiAveragePrice[ posFigi ] = pos.getAveragePositionPrice();

        // pos.getBlocked(); - Хз, что такое, заблокировано по какой-то причине
        // Возможно, выставлена лимитная заявка(и)
        // В любом случае, оно всё равно в портфельчике

    }



    //bool dictionaryGetValue( const std::map<K,V> &d, V &vOut, const K &k )

    std::map< QString, std::set<QString> > foundOperationTypes   ; // by FIGI, set of ops
    std::map< QString, std::set<QString> > foundOperationStatuses; // by FIGI, set of ops


    typedef std::map< QString, marty::Decimal >   CurrencyTotalCounter;

    std::map< QString, CurrencyTotalCounter >     figiBalance;
    std::map< QString, CurrencyTotalCounter >     figiCommission;
    std::map< QString, CurrencyTotalCounter >     figiDevidends;
    std::map< QString, CurrencyTotalCounter >     figiDevidendsTax;

    QDateTime curTime   = QDateTime::currentDateTime();
    QDateTime startDate = curTime.addYears(-20); //curTime.date().addYears(-1);

    for( auto figi : balanceConfig.figis ) 
    {
        cout << "Processing " << figi;
        if (tkf::dictionaryGetValue(tickerToFigi, figi, figi)) {}
        else if (tkf::dictionaryGetValue(isinToFigi, figi, figi)) {}
    
        cout << ", FIGI: " << figi << endl;

        auto // OperationsResponse
        operationsRes = pOpenApi->operations( startDate, curTime, figi );
      
        operationsRes->join();
        tkf::checkAbort(operationsRes);
      
        auto // std::vector< tkf::Operation >
        operations = tkf::makeOrderedVectorFromList( operationsRes->value.getPayload().getOperations()
                                                   //, tkf::WithGetDateGreater<tkf::Operation>()
                                                   , tkf::WithGetDateLess<tkf::Operation>()
                                                   );

        for( const auto &op : operations )
        {
            if (!op.isSet() || !op.isValid())
                continue;
     
            QString operationTypeStr   = op.getOperationType().asJson().toUpper();
            QString operationStatusStr = op.getStatus().asJson().toUpper();

            foundOperationTypes[figi].insert(operationTypeStr);
            foundOperationStatuses[figi].insert(operationStatusStr);

            if (operationStatusStr!="DONE")
                continue;

            cout << op;
            cout << "------------------------------------------------" << endl;

            // BROKERCOMMISSION, BUY, DIVIDEND, SELL, TAXDIVIDEND
            // BBG0013HGFT4 (USD000UTSTOM) : BROKERCOMMISSION, BUY, BUYCARD, SELL

            // BUYCARD - появилось в операциях с USD000UTSTOM, по сути - похоже на простой BUY

            // Плохо, что комиссия идёт отдельным полем в операции, и отдельно идёт записью в списке операций, 
            // и их никак не связать, только по времени исполнения, но это не гарантированно.

            //QString figi = op.getFigi().asJson().toUpper();
            QString currencyName = op.getCurrency().asJson().toUpper();

            if (operationTypeStr=="BUY" || operationTypeStr=="BUYCARD" || operationTypeStr=="SELL")
            {
                figiBalance[figi][currencyName] += op.getPayment();
            }
            else if (operationTypeStr=="BROKERCOMMISSION")
            {
                figiCommission[figi][currencyName] += op.getPayment();
            }
            else if (operationTypeStr=="DIVIDEND")
            {
                figiDevidends[figi][currencyName] += op.getPayment();
            }
            else if (operationTypeStr=="TAXDIVIDEND")
            {
                figiDevidendsTax[figi][currencyName] += op.getPayment();
            }

        } // for( const auto &op : operations )

        if (slowMode)
            QTest::qWait(5000);
        else
            QTest::qWait(1000);


    } // for( auto figi : balanceConfig.figis ) 

    

    cout << endl;
    cout << "----------------------------------------------------------------------------" << endl;
    cout << "Found Operation Types" << endl;
    printMapOfFigiSets( foundOperationTypes, figiToTicker );

    cout << endl;
    cout << "----------------------------------------------------------------------------" << endl;
    cout << "Found Operation Statuses" << endl;
    printMapOfFigiSets( foundOperationStatuses, figiToTicker );


    //#define DETAILED_REPORT


    for( auto figi : balanceConfig.figis ) 
    {
        cout << endl;
        cout << "----------------------------------------------------------------------------" << endl;

        if (tkf::dictionaryGetValue(tickerToFigi, figi, figi)) {}
        else if (tkf::dictionaryGetValue(isinToFigi, figi, figi)) {}
    
        cout << "Instrument Balance Summary: " << figi << " (" << tkf::dictionaryGetValue(figiToTicker, figi) << ") - " << tkf::dictionaryGetValue(figiToName, figi) << endl;

        std::set<QString> figiCurrencies;
        std::map< QString, marty::Decimal > figiCurrencyTotalBalance;

        const std::map< QString, marty::Decimal > & figiCurrencyBalance               = figiBalance[figi];
        const std::map< QString, marty::Decimal > & figiCurrencyCommission            = figiCommission[figi];
        const std::map< QString, marty::Decimal > & figiCurrencyDevidends             = figiDevidends[figi];
        const std::map< QString, marty::Decimal > & figiCurrencyDevidendsTax          = figiDevidendsTax[figi];
        const std::map< QString, marty::Decimal > & portfolioFigiCurrencyAveragePrice = portfolioFigiAveragePrice[figi];


        tkf::getMapKeys( figiCurrencyBalance     , figiCurrencies );
        tkf::getMapKeys( figiCurrencyCommission  , figiCurrencies );
        tkf::getMapKeys( figiCurrencyDevidends   , figiCurrencies );
        tkf::getMapKeys( figiCurrencyDevidendsTax, figiCurrencies );
        tkf::getMapKeys( portfolioFigiCurrencyAveragePrice, figiCurrencies );

        std::map< QString, marty::Decimal > portfolioFigiCurrencyAverageCost;

        #if defined(DETAILED_REPORT)
        cout << "Instrument Portfolio Balance" << endl;
        cout << "Balance: " << portfolioFigiBalance[figi] << endl;
        cout << "Instrument Average Portfolio Cost" << endl;
        #endif


        QString        instrumentCurrency; // Also used for candles
        marty::Decimal instrumentPositionsAverageCost;

        std::set<QString>::const_iterator figiCurrencyIt = figiCurrencies.begin();
        for( ; figiCurrencyIt!=figiCurrencies.end(); ++figiCurrencyIt)
        {
            const QString &currentCurrency = *figiCurrencyIt;

            std::map< QString, marty::Decimal >::const_iterator avgPriceByCurrencyIt /* curIt */  = portfolioFigiCurrencyAveragePrice.find(currentCurrency);
            if (avgPriceByCurrencyIt == portfolioFigiCurrencyAveragePrice.end())
                continue;

            std::map< QString, marty::Decimal   >::const_iterator figiBalanceIt = portfolioFigiBalance.find(figi);
            if (figiBalanceIt == portfolioFigiBalance.end())
                continue;

            instrumentCurrency = currentCurrency;

            auto cost = avgPriceByCurrencyIt->second * figiBalanceIt->second;

            // figiCurrencyTotalBalance[currentCurrency] += cost;

            //portfolioFigiCurrencyAverageCost[currentCurrency] = cost;

            instrumentPositionsAverageCost = cost;

            #if defined(DETAILED_REPORT)
            cout << "    " << currentCurrency << " : " << figiBalanceIt->second << " x " << curIt->second << " = " << cost << endl;
            #endif
        }



        std::map< QString, marty::Decimal   >::const_iterator figiBalanceIt = portfolioFigiBalance.find(figi);
        if (figiBalanceIt != portfolioFigiBalance.end())
        {
            // figiBalanceIt->second

            #if defined(DETAILED_REPORT)
            cout << "Instrument Current Portfolio Cost" << endl;
            #endif

            // Считаем среднюю текущую цену инструмента по данным свечи
            // auto avgCurrentPrice = ( figiCandleIt->second.getO() + figiCandleIt->second.getC() ) / marty::Decimal(2);

            // Считаем текущую стоимость портфеля
            auto cost = instrumentPositionsAverageCost  /* portfolioFigiAveragePrice[figi][instrumentCurrency] */
                      + portfolioFigiExpectedYield[figi][instrumentCurrency];


            // Стоимость инструментов в портфеле прибавляем к балансу
            figiCurrencyTotalBalance[instrumentCurrency] += cost;
           
            #if defined(DETAILED_REPORT)
            cout << "    " << instrumentCurrency << " : " << figiBalanceIt->second << " x " << avgCurrentPrice << " = " << cost << endl;
            #endif

            // auto deltaCost = cost - instrumentPositionsAverageCost;
            auto deltaCost = portfolioFigiExpectedYield[figi][instrumentCurrency];

            // instrumentPositionsAverageCost - 100%

            // auto deltaCostPercent = 100 * deltaCost / instrumentPositionsAverageCost;
            //auto deltaCostPercent = marty::Decimal(100) * deltaCost / instrumentPositionsAverageCost;
            marty::Decimal deltaCostPercent = 0;
            if (instrumentPositionsAverageCost!=0)
                deltaCostPercent = deltaCost.getPercentOf(instrumentPositionsAverageCost);

            cout << "Profit (current)" << endl
                 << "    " << instrumentCurrency << " : " << deltaCost << " - " << deltaCostPercent << "%" << endl;
        }


        auto processCurrencies = [&figiCurrencies, &figiCurrencyTotalBalance]( const std::map< QString, marty::Decimal > &currencyDecimals ) -> void
             {
                 std::set<QString>::const_iterator figiCurrencyIt = figiCurrencies.begin();

                 for (; figiCurrencyIt!=figiCurrencies.end(); ++figiCurrencyIt)
                 {
                     const QString &currentCurrency = *figiCurrencyIt;

                     std::map< QString, marty::Decimal >::const_iterator curIt = currencyDecimals.find(currentCurrency);

                     if (curIt == currencyDecimals.end())
                         continue;

                     figiCurrencyTotalBalance[currentCurrency] += curIt->second;

                     #if defined(DETAILED_REPORT)
                     cout << "    " << currentCurrency << " : " << curIt->second << endl;
                     #endif
                 }

             };

        #if defined(DETAILED_REPORT)
        cout << "Instrument Buy/Sale Balance" << endl;
        #endif
        processCurrencies( figiCurrencyBalance );

        #if defined(DETAILED_REPORT)
        cout << "Instrument Commission Balance" << endl;
        #endif
        processCurrencies( figiCurrencyCommission );

        #if defined(DETAILED_REPORT)
        cout << "Instrument Devidends Balance" << endl;
        #endif
        processCurrencies( figiCurrencyDevidends );

        #if defined(DETAILED_REPORT)
        cout << "Instrument Devidends Tax Balance" << endl;
        #endif
        processCurrencies( figiCurrencyDevidendsTax );


        cout << "Instrument Total Summary Balance" << endl;

        //std::set<QString>::const_iterator 
        figiCurrencyIt = figiCurrencies.begin();

        for (; figiCurrencyIt!=figiCurrencies.end(); ++figiCurrencyIt)
        {
            const QString &currentCurrency = *figiCurrencyIt;

            std::map< QString, marty::Decimal >::const_iterator curIt = figiCurrencyTotalBalance.find(currentCurrency);

            if (curIt == figiCurrencyTotalBalance.end())
                continue;

            cout << "    " << currentCurrency << " : " << curIt->second << endl;
        }


    }

    cout << endl;
    cout << "----------------------------------------------------------------------------" << endl;

        //QTest::qWait(1000);



    
    return 0;
}



