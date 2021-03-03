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
    auto balanceConfigFullFileName = lookupForConfigFile( "balance.properties" , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );

    qDebug().nospace().noquote() << "Log  Config File: "<< logConfigFullFileName  ;
    qDebug().nospace().noquote() << "API  Config File: "<< apiConfigFullFileName  ;
    qDebug().nospace().noquote() << "Auth Config File: "<< authConfigFullFileName ;
    qDebug().nospace().noquote() << "DB   Config     : "<< dbConfigFullFileName   ;
    qDebug().nospace().noquote() << "Balance Config  : "<< balanceConfigFullFileName;

    auto apiConfig     = tkf::ApiConfig    ( apiConfigFullFileName  );
    auto authConfig    = tkf::AuthConfig   ( authConfigFullFileName );
    auto balanceConfig = tkf::BalanceConfig( balanceConfigFullFileName );
    //auto balance_config.h


    QSharedPointer<tkf::DatabaseConfig> pDatabaseConfig = QSharedPointer<tkf::DatabaseConfig>( new tkf::DatabaseConfig(dbConfigFullFileName, tkf::DatabasePlacementStrategyDefault()) );
    QSharedPointer<tkf::LoggingConfig>  pLoggingConfig  = QSharedPointer<tkf::LoggingConfig> ( new tkf::LoggingConfig(logConfigFullFileName) );
    auto loggingConfig = *pLoggingConfig;


    qDebug().nospace().noquote() << "DB name: " << pDatabaseConfig->dbFilename;

    QSharedPointer<QSqlDatabase> pSqlDb = QSharedPointer<QSqlDatabase>( new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE")) );
    pSqlDb->setDatabaseName( pDatabaseConfig->dbFilename );

    if (!pSqlDb->open())
    {
      qDebug() << pSqlDb->lastError().text();
      return 0;
    }

    QSharedPointer<tkf::IDatabaseManager> pDbMan = tkf::createDatabaseManager( pSqlDb, pDatabaseConfig, pLoggingConfig );

    pDbMan->applyDefDecimalFormatFromConfig( *pDatabaseConfig );



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


    std::map< QString, int > figiToId   = pDbMan->getDictionaryFromTable  ( "MARKET_INSTRUMENT", "FIGI,ID"  );
    std::map< QString, int > tickerToId = pDbMan->getDictionaryFromTable  ( "MARKET_INSTRUMENT", "TICKER,ID");
    std::map< QString, int > isinToId   = pDbMan->getDictionaryFromTable  ( "MARKET_INSTRUMENT", "ISIN,ID"  );
    std::map< int, QString > idToName   = pDbMan->getIdToFieldMapFromTable( "MARKET_INSTRUMENT", "ID,NAME"  );

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

    for( auto pos : portfolio )
    {
        auto posFigi = pos.getFigi().toUpper();
        portfolioFigiBalance[ posFigi ]      = pos.getBalance();

        tkf::MoneyAmount moneyAmount = pos.getAveragePositionPrice();
        QString moneyAmountCurrency  = moneyAmount.getCurrency().asJson().toUpper();

        portfolioFigiAveragePrice[posFigi][moneyAmountCurrency] = moneyAmount.getValue();

        //portfolioFigiAveragePrice[ posFigi ] = pos.getAveragePositionPrice();

        // pos.getBlocked(); - Хз, что такое, заблокировано по какой-то причине
        // Возможно, выставлена лимитная заявка(и)
        // В любом случае, оно всё равно в портфельчике

    }


    //#if defined(I_AM_A_DEBILOID)

        std::map< QString, tkf::Candle > figiLastCandle; // Последняя пятиминутная свеча для данной фиги - 
                                                         // хз, я не не понял/не нашел, как получить текущую стоимость 
                                                         // инструмента, поэтому буду использовать среднее открытия/закрытия свечи
       
        // Получаем текущую цену по инструментам

        // Когда я выводил портфолио (_tkf_portfolio.cpp), я почему-то не стал выводить стоимость инструмента, которая там была.
        // И потом, когда я стал писать этот тест, я пребывал в уверенности, что нет способа получить текущую стоимость.
        // Поэтому наколбасил кучу кода получения стоимости через свечи.
        // Но потом таки дошло.
        // Ну, нет hood'а без бобра - зато на свечках потренировался

        // На самом деле в инструменте фигурирует средняя стоимость, которая расчитывается потомцене покупки.
        // Так что текущую стоимость таки по свечкам определяем

        for( auto figi : balanceConfig.figis ) 
        {
            cout << "Processing " << figi;
            if (tkf::dictionaryGetValue(tickerToFigi, figi, figi)) {}
            else if (tkf::dictionaryGetValue(isinToFigi, figi, figi)) {}
       
            cout << ", FIGI: " << figi << endl;
       
            QDateTime curDateTime = QDateTime::currentDateTime().toUTC();
       
            std::vector<QDateTime> candleDateTimes;
       
            // Самые последние - в начале
            candleDateTimes.push_back(curDateTime);
            {
                QDate todayBeginDate = curDateTime.date();
                QDateTime todayBeginDateTime; todayBeginDateTime.setDate(todayBeginDate);
                candleDateTimes.push_back(todayBeginDateTime);
            }
       
            for( unsigned i=0; i!=15; ++i )
            {
                QDate prevDate = curDateTime.date().addDays(-1);
                QDateTime prevDateTime; prevDateTime.setDate(prevDate);
                candleDateTimes.push_back(prevDateTime);
                curDateTime = prevDateTime;
            }
       
            std::reverse( candleDateTimes.begin(), candleDateTimes.end() );
       
            // Самые последние - теперь в конце
       
            // Получаем пятиминутные свечи за последние 12 дней (самые длинные каникулы меньше, поэтому что-то да получим)
            // Это нужно, чтобы узнать актуальную стоимость инструмента
       
            std::vector<tkf::Candle> instrumentCandles;
       
            for( unsigned i=0; i!=(candleDateTimes.size()-1); ++i )
            {
                auto // CandlesResponse
                candlesRes = pOpenApi->marketCandles( figi, candleDateTimes[i], candleDateTimes[i+1], "5MIN" );
               
                candlesRes->join();
       
                tkf::checkAbort(candlesRes);
       
                auto candles = tkf::makeVectorFromList(candlesRes->value.getPayload().getCandles());
       
                instrumentCandles.insert( instrumentCandles.end(), candles.begin(), candles.end() );
       
                // 1 запрос - 0.2 секунды (примерно, на практике), всего 3 секунды чистого времени
                // Притормаживаем на 0.3 секунды, чтобы уложиться в лимит 120 запросов в минуту
                QTest::qWait(300);
       
                // Итого 7.5 секунды на инструмент
            }
       
            // Сортируем по убыванию - первая свечка - самая последняя по дате
            std::stable_sort( instrumentCandles.begin(), instrumentCandles.end(), tkf::WithGetTimeGreater<tkf::Candle>() );
       
            if (!instrumentCandles.empty())
            {
                cout << "First candle DateTime: " << instrumentCandles[0]                         .getTime() << endl;
                cout << "Last  candle DateTime: " << instrumentCandles[instrumentCandles.size()-1].getTime() << endl;
       
                cout << "----------------------------------------------------------------------------" << endl;
                cout << endl << endl << endl;
       
                figiLastCandle[figi] = instrumentCandles[0];
            }
            else
            {
                // Ooops, инструмент не торговался последние 15 дней
                // Пока мы работаем без базы по свечам и остальному, всё берём в онлайне
                // поэтому сорян
            }
       
        } // for( auto figi : balanceConfig.figis ) 

    //#endif
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

        cout << "Instrument Portfolio Balance" << endl;
        cout << "Balance: " << portfolioFigiBalance[figi] << endl;
        cout << "Instrument Average Portfolio Cost" << endl;


        QString        instrumentCurrency; // Also used for candles
        marty::Decimal instrumentPositionsAverageCost;

        std::set<QString>::const_iterator figiCurrencyIt = figiCurrencies.begin();
        for( ; figiCurrencyIt!=figiCurrencies.end(); ++figiCurrencyIt)
        {
            const QString &currentCurrency = *figiCurrencyIt;

            std::map< QString, marty::Decimal >::const_iterator curIt = portfolioFigiCurrencyAveragePrice.find(currentCurrency);
            if (curIt == portfolioFigiCurrencyAveragePrice.end())
                continue;

            std::map< QString, marty::Decimal   >::const_iterator figiBalanceIt = portfolioFigiBalance.find(figi);
            if (figiBalanceIt == portfolioFigiBalance.end())
                continue;

            instrumentCurrency = currentCurrency;

            auto cost = curIt->second * figiBalanceIt->second;

            // figiCurrencyTotalBalance[currentCurrency] += cost;

            //portfolioFigiCurrencyAverageCost[currentCurrency] = cost;

            instrumentPositionsAverageCost = cost;

            cout << "    " << currentCurrency << " : " << figiBalanceIt->second << " x " << curIt->second << " = " << cost << endl;
        }


        std::map< QString, tkf::Candle >::const_iterator figiCandleIt = figiLastCandle.find(figi);
        if (!instrumentCurrency.isEmpty() && figiCandleIt != figiLastCandle.end())
        {
            std::map< QString, marty::Decimal   >::const_iterator figiBalanceIt = portfolioFigiBalance.find(figi);
            if (figiBalanceIt != portfolioFigiBalance.end())
            {
                // figiBalanceIt->second

                cout << "Instrument Current Portfolio Cost" << endl;
               
                auto avgCurrentPrice = ( figiCandleIt->second.getO() + figiCandleIt->second.getC() ) / marty::Decimal(2);

                auto cost = avgCurrentPrice * figiBalanceIt->second;

                figiCurrencyTotalBalance[instrumentCurrency] += cost;
               
                cout << "    " << instrumentCurrency << " : " << figiBalanceIt->second << " x " << avgCurrentPrice << " = " << cost << endl;

                auto deltaCost = cost - instrumentPositionsAverageCost;

                // instrumentPositionsAverageCost - 100%

                // auto deltaCostPercent = 100 * deltaCost / instrumentPositionsAverageCost;
                //auto deltaCostPercent = marty::Decimal(100) * deltaCost / instrumentPositionsAverageCost;
                auto deltaCostPercent = deltaCost.getPercentOf(instrumentPositionsAverageCost);

                cout << "Profit (current)" << endl
                     << "    " << instrumentCurrency << " : " << deltaCost << " - " << deltaCostPercent << "%" << endl;
            }
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

                     cout << "    " << currentCurrency << " : " << curIt->second << endl;
                 }

             };

        cout << "Instrument Buy/Sale Balance" << endl;
        processCurrencies( figiCurrencyBalance );

        cout << "Instrument Commission Balance" << endl;
        processCurrencies( figiCurrencyCommission );

        cout << "Instrument Devidends Balance" << endl;
        processCurrencies( figiCurrencyDevidends );

        cout << "Instrument Devidends Tax Balance" << endl;
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



