/*! \file
    \brief Configs lookup test

 */

#include <iostream>
#include <exception>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <iterator>
#include <map>
#include <set>
#include <optional>

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
#include "invest_openapi/qt_time_helpers.h"
#include "invest_openapi/ioa_utils.h"
#include "invest_openapi/ioa_ostream.h"
#include "invest_openapi/ioa_db_dictionaries.h"

#include "invest_openapi/console_break_helper.h"

INVEST_OPENAPI_MAIN()
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("_tkf_historical_candles");
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


    auto logConfigFullFileName     = lookupForConfigFile( "logging.properties" , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto apiConfigFullFileName     = lookupForConfigFile( "config.properties"  , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto authConfigFullFileName    = lookupForConfigFile( "auth.properties"    , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto dbConfigFullFileName      = lookupForConfigFile( "database.properties", lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    auto histCandlesFullFileName   = lookupForConfigFile( "historical_candles.properties", lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );
    //auto balanceConfigFullFileName = lookupForConfigFile( "balance.properties" , lookupConfSubfolders, FileReadable(), QCoreApplication::applicationDirPath(), true, -1 );

    qDebug().nospace().noquote() << "Log  Config File  : "<< logConfigFullFileName   ;
    qDebug().nospace().noquote() << "API  Config File  : "<< apiConfigFullFileName   ;
    qDebug().nospace().noquote() << "Auth Config File  : "<< authConfigFullFileName  ;
    qDebug().nospace().noquote() << "DB   Config       : "<< dbConfigFullFileName    ;
    qDebug().nospace().noquote() << "HistCndl Cfg File : "<< histCandlesFullFileName ;
    //qDebug().nospace().noquote() << "Balance Config  : "<< balanceConfigFullFileName;


    auto apiConfig     = tkf::ApiConfig    ( apiConfigFullFileName  );
    auto authConfig    = tkf::AuthConfig   ( authConfigFullFileName );
    //auto balanceConfig = tkf::BalanceConfig( balanceConfigFullFileName );
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

    QSharedPointer<tkf::IDatabaseManager> pDbMan = tkf::createMainDatabaseManager( pSqlDb, pDatabaseConfig, pLoggingConfig );

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

    tkf::DatabaseDictionaries dicts = tkf::DatabaseDictionaries(pDbMan);


    int stockExchangeId = dicts.getStockExangeListIdChecked("moex");



    std::set<QString> explicitFigisForUpdate;
    {
        QSettings settings(histCandlesFullFileName, QSettings::IniFormat);
        auto instrumentList = settings.value("historical-candles-update-list" ).toStringList();

        for( const auto instrument : instrumentList )
        {
            QString figi = dicts.findFigiByAnyIdString(instrument);
            if (figi.isEmpty())
                continue;

            explicitFigisForUpdate.insert(figi);
        }
    }



    auto instrumentCandlesTableFields = QString("INSTRUMENT_ID,STOCK_EXCHANGE_ID,CANDLE_RESOLUTION_ID,CANDLE_DATE_TIME,CURRENCY_ID,OPEN_PRICE,CLOSE_PRICE,HIGH_PRICE,LOW_PRICE,VOLUME");
    auto instrumentCandlesTableFieldsVec = tkf::convertToQVectorOfQStrings(instrumentCandlesTableFields);


    console_helpers::SimpleHandleCtrlC ctrlC;

    /*
        Какая нужна статистика по инструменту?

        Время, затраченное на получение данных - слишком большое время говорит о том, что с инструментом что-то не ладно
    
    */

    QElapsedTimer instrumentCandlesTimer;

    QElapsedTimer singleRequestTimer;
    singleRequestTimer.start();

    unsigned singleRequestElapsedTime = 0;

    std::set<QString> discontinuedFigis;
    std::set<QString> erroneousFigis;
    std::map<QString, std::uint64_t> figiTimes;


    int instrumentId     = dicts.getInstrumentIdBegin( );
    int instrumentIdEnd  = dicts.getInstrumentIdEnd( );

    for( ; instrumentId!=instrumentIdEnd && !ctrlC.isBreaked(); ++instrumentId )
    {
        if (!dicts.isValidId(instrumentId))
            continue;

        // QElapsedTimer instrumentTimer;
        // instrumentTimer.start();
        // auto instrumentTimeSummary = instrumentTimer.elapsed();


        QString figi = dicts.getInstrumentById( instrumentId );
        if (figi.isEmpty())
            continue; // There is a GAP found in instruments enumeration


        // Check for exact taken figis
        if (!explicitFigisForUpdate.empty())
        {
            if (explicitFigisForUpdate.find(figi)==explicitFigisForUpdate.end())
                continue; // Not found in explicitly taken list
        }


        if (discontinuedFigis.find(figi)!=discontinuedFigis.end())
        {
            cout << "!<>! FIGI is delisted" << endl;
            continue;
        }

        if (erroneousFigis.find(figi)!=erroneousFigis.end())
        {
            cout << "!<>! FIGI is erroneous" << endl;
            continue;
        }


        QString ticker         = dicts.getTickerByFigiChecked(figi);
        QString instrumentName = dicts.getNameByFigiChecked(figi);

        instrumentCandlesTimer.restart();

        int instrumentCurrencyId = dicts.getInstrumentCurrencyId( instrumentId );

        

        cout << endl << "----------------------------------------------------------------------------" << endl;
        cout << "!!!!! Processing instrument " << instrumentId << ", FIGI: " << figi << " (" << ticker << ") - " << instrumentName << endl;


        // Начинаем по порядку с самых жирных свечей - они теоретически помогут нам найти стартовые даты для
        // интервалов большего разрешния, чтобы не искать лишнего вхолостую

        // Есть нюанс - свечи высокого разрешения могут не существовать, даже если есть свечи низкого разрешения

        // Стартовую дату я ведь так и ищу сначала через месячные свечи, потом через суточные. А вот минутных свечей
        // много где не хватает. Так что, наверное, не стоит пока заморачиваться.

        int candleResolutionBeginId  = dicts.getCandleResolutionIdBegin( );
        int candleResolutionEndId    = dicts.getCandleResolutionIdEnd( );

        int candleResolutionRBeginId = candleResolutionEndId   - 1;
        int candleResolutionREndId   = candleResolutionBeginId - 1;

        singleRequestTimer.restart();


        std::map<int, QDate> candleIntervalStartDates;
        candleIntervalStartDates[8] = qt_helpers::dateFromDbString("2019-01-01"); // HOUR
        candleIntervalStartDates[7] = qt_helpers::dateFromDbString("2019-01-01"); // 30MIN
        candleIntervalStartDates[6] = qt_helpers::dateFromDbString("2019-01-01"); // 15MIN
        candleIntervalStartDates[5] = qt_helpers::dateFromDbString("2019-01-01"); // 10MIN
        candleIntervalStartDates[4] = qt_helpers::dateFromDbString("2019-01-01"); // 5MIN
        candleIntervalStartDates[3] = qt_helpers::dateFromDbString("2019-01-01"); // 3MIN
        candleIntervalStartDates[2] = qt_helpers::dateFromDbString("2019-01-01"); // 2MIN
        candleIntervalStartDates[1] = qt_helpers::dateFromDbString("2019-01-01"); // 1MIN


        for( int candleResolutionId = candleResolutionRBeginId
           ; candleResolutionId!=candleResolutionREndId && !ctrlC.isBreaked()
           ; --candleResolutionId)
        {

            if (discontinuedFigis.find(figi)!=discontinuedFigis.end())
                break;
           
            if (erroneousFigis.find(figi)!=erroneousFigis.end())
                break;


            QString candleResolution = dicts.getCandleResolutionById(candleResolutionId);
            if (!dicts.isValidId(candleResolution))
               continue;

            if (discontinuedFigis.find(figi)!=discontinuedFigis.end())
            {
                cout << "!<>! FIGI is delisted (detected in inner loop)" << endl;
                break;;
            }


            QString candleIntervalMin = dicts.getCandleResolutionIntervalMinById(candleResolutionId);
            QString candleIntervalMax = dicts.getCandleResolutionIntervalMaxById(candleResolutionId);
            QString candleIntervalRec = dicts.getCandleResolutionIntervalRecById(candleResolutionId);

            cout << "      " << "Processing candles with resolution " << candleResolution 
                 << ", interval min: " << candleIntervalMin
                 << ", interval max: " << candleIntervalMax
                 << ", interval rec: " << candleIntervalRec
                 << endl;

            QElapsedTimer candleLastDateLookupTimer;
            candleLastDateLookupTimer.start();

            bool atLeastOneCandleFound = false;

            bool foundInDb = false;


            // Нужно найти дату последней имеющейся свечи данного интервала
            // INSTRUMENT_CANDLES, поля INSTRUMENT_ID, STOCK_EXCHANGE_ID, CANDLE_RESOLUTION_ID - фильтр
            // Выбираем поле CANDLE_DATE_TIME с максимальным значением

            QString selectCandleDateTimeFromInstrumentCandlesQueryText 
                        = pDbMan->makeSimpleSelectQueryText( "INSTRUMENT_CANDLES"
                                                           , "INSTRUMENT_ID,STOCK_EXCHANGE_ID,CANDLE_RESOLUTION_ID" // whereNames
                                                           , QString("%1,%2,%3").arg(instrumentId).arg(stockExchangeId).arg(candleResolutionId) // whereVals
                                                           , "CANDLE_DATE_TIME" // fields to select
                                                           );

            QString selectLastDateQueryText 
                        = pDbMan->makeSelectSingleValueQuery( selectCandleDateTimeFromInstrumentCandlesQueryText , "CANDLE_DATE_TIME", true /* true for last, false for first */ );


            //bool papyNotGood = true;

            QDateTime dtLastCandleDate;

            // std::optional - https://habr.com/ru/post/372103/
            // Но на самом деле он не нужен

            auto resVec = pDbMan->execSelectQueryReturnFirstRow( selectLastDateQueryText );

            if ( !resVec.empty() && !resVec.front().isEmpty() )
            {
                dtLastCandleDate = qt_helpers::dateTimeFromDbString( resVec.front() );
                cout << "/// Starting date found in INSTRUMENT_CANDLES: " << dtLastCandleDate << endl;

                foundInDb = true;

                // Чисто теоретически, у нас мог бы быть очень длительный интервал, когда мы не обновляли базу
                // И тогда мы тут зафейлимся. Но я пока хоть раз в месяц, да обновляю её

                // Поэтому, если в базе последние данные старше 3 месяцев - тогоэто какая-то проблема.
                // И этот инструмент больше не пробуем.

                // Но надо будет а) где-то добавить опцию, чтобы данной проверки не делалось
                //               б) завести таблицу delisted инструментов, и из неё брать признак - 
                //                  нужно ли пытаться обновлять свечи по инструменту


                QDateTime dtNow = QDateTime::currentDateTime();
                dtNow = dtNow.toUTC();

                QDateTime dtAgeLim = qt_helpers::dtAddTimeInterval( 3 /* howManyAdds */, dtNow, QString("-1MONTH") );
                //QDateTime dtAgeLim = dtNow;
                cout << "/// Age limit is : " << dtAgeLim << endl;

                if (dtLastCandleDate < dtAgeLim)
                {
                    // cout << "!<>! FIGI is delisted" << endl;
                    cout << "!!<>!! Mark FIGI as delisted" << endl;
                    discontinuedFigis.insert(figi);
                    break;
                }
                else
                {
                    cout << "Instrument allowed for update" << endl;
                }

                // std::set<QString> discontinuedFigis;

                // К найденному значению надо прибавить значение интервала свечи, чтобы не искать с того же интервала
                //dtLastCandleDate = qt_helpers::dtAddTimeInterval( dtLastCandleDate, candleResolution );

                // Немного отступаем назад, потому что последний интервал может быть некорректным
                // он мог быть корректным на момент запроса, но охватывать не полный период

                //QString candleResolutionStr = dicts.getCandleResolutionByIdChecked( candleResolutionId );

                // На всякий отступаем на два минимальных интервала.
                QString adjustmentStr = "-" + candleIntervalMin;
                cout << "    Adjustment                               : " << adjustmentStr << endl;
                //dtLastCandleDate = qt_helpers::dtAddTimeInterval( dtLastCandleDate, adjustmentStr );
                //dtLastCandleDate = qt_helpers::dtAddTimeInterval( dtLastCandleDate, adjustmentStr );

                dtLastCandleDate = qt_helpers::dtAddTimeInterval( 2 /* howManyAdds */, dtLastCandleDate, adjustmentStr );

                cout << "    Starting date adjusted                   : " << dtLastCandleDate << endl;

                /*
                if (candleResolutionId<=9)
                {
                    int candleResolutionBeginId  = dicts.getCandleResolutionIdBegin( );
                    // dtLastCandleDate = qt_helpers::dtAddTimeInterval( dtLastCandleDate, "-1DAY" ); // Это слишком долго
                }
                else
                {
                    dtLastCandleDate = qt_helpers::dtAddTimeInterval( dtLastCandleDate, "-" + candleIntervalMin );
                    //dtLastCandleDate = qt_helpers::dtAddTimeInterval( dtLastCandleDate, "-" + candleIntervalMin );
                }
                */

                // А вот это категорически не нужно
                #if 0
                QDate    newDate = dtLastCandleDate.date();
                auto    zeroTime = QTime(0 /* h */, 0 /* m */ , 0 /* s */, 0 /* ms */ );
                QTimeZone  qtz   = dtLastCandleDate.timeZone();

                dtLastCandleDate = QDateTime( newDate, zeroTime, qtz );
                #endif
            }



            //QDateTime dtLastCandleDateFromConst;
            //QDateTime dtLastCandleDateFromListingDate;

            if (!tkf::isQtValidNotNull(dtLastCandleDate))
            {
                // В таблице не нашли, значит, надо искать с начала

                QDateTime dtLastCandleDateFromConst;
               
                // Если дата не найдена в таблице, ищем дату среди констант
                //if (!tkf::isQtValidNotNull(dtLastCandleDate))
                {
                    std::map<int, QDate>::const_iterator cdtIt = candleIntervalStartDates.find(candleResolutionId);
                    if (cdtIt != candleIntervalStartDates.end())
                    {
                        auto date = cdtIt->second;
                        auto zeroTime = QTime(0 /* h */, 0 /* m */ , 0 /* s */, 0 /* ms */ );
                        dtLastCandleDateFromConst = QDateTime(date, zeroTime, Qt::UTC); // .setDate(date);
                    }
                }
               
               
               
                QDateTime dtLastCandleDateFromListingDate;
               
                //if (!tkf::isQtValidNotNull(dtLastCandleDate))
                {
                    // Если дата не найдена в таблице, ищем дату листинга инструмента на бирже
                    // INSTRUMENT_LISTING_DATES, поля INSTRUMENT_ID, STOCK_EXCHANGE_ID - фильтр
                    // Выбираем поле LISTING_DATE
               
                    cout << "      Looking in INSTRUMENT_LISTING_DATES" << endl;
               
                    QString selectQueryText 
                            = pDbMan->makeSimpleSelectQueryText( "INSTRUMENT_LISTING_DATES"
                                                               , "INSTRUMENT_ID,STOCK_EXCHANGE_ID" // whereNames
                                                               , QString("%1,%2").arg(instrumentId).arg(stockExchangeId) // whereVals
                                                               , "LISTING_DATE" // fields to select
                                                               );
               
                    selectLastDateQueryText 
                            = pDbMan->makeSelectSingleValueQuery( selectQueryText, "LISTING_DATE", true /* true for last, false for first */ );
               
               
                    auto resVec = pDbMan->execSelectQueryReturnFirstRow( selectLastDateQueryText );
               
                    if ( !resVec.empty() && !resVec.front().isEmpty() )
                    {
                        //!!!
                        auto date = qt_helpers::dateFromDbString( resVec.front() );
                        //auto date = qt_helpers::dateFromDbString( "2019-02-28" );
                        auto zeroTime = QTime(0 /* h */, 0 /* m */ , 0 /* s */, 0 /* ms */ );
                        dtLastCandleDateFromListingDate = QDateTime(date, zeroTime, Qt::UTC); // .setDate(date);
                    }
                }

            //QDateTime dtLastCandleDateFromConst;
            //QDateTime dtLastCandleDateFromListingDate;

                // if (!tkf::isQtValidNotNull(dtLastCandleDate))

                if (!tkf::isQtValidNotNull(dtLastCandleDateFromConst) && !tkf::isQtValidNotNull(dtLastCandleDateFromListingDate))
                {
                    // Обе даты не установлены, берем за последний год + пара месяцев
                    dtLastCandleDate = qt_helpers::dateTimeFromDbString("2020-01-01 00:00:00.000");
                }
                else if (tkf::isQtValidNotNull(dtLastCandleDateFromConst) && tkf::isQtValidNotNull(dtLastCandleDateFromListingDate))
                {
                    // Обе даты установлены, берём ту, которая позже
                    dtLastCandleDate = (dtLastCandleDateFromConst > dtLastCandleDateFromListingDate) ? dtLastCandleDateFromConst : dtLastCandleDateFromListingDate;
                }
                else if (tkf::isQtValidNotNull(dtLastCandleDateFromConst))
                {
                    dtLastCandleDate = dtLastCandleDateFromConst;
                }
                else
                {
                    dtLastCandleDate = dtLastCandleDateFromListingDate;
                }
            }


            auto elapsed = (unsigned)candleLastDateLookupTimer.restart();

            cout << "      " << "DB queries performed in " << elapsed << " ms" << endl;

            if (!tkf::isQtValidNotNull(dtLastCandleDate))
            {
                cout << "      " << "Starting date not found at all" << endl;
                continue;
            }

            // Делаем std::set компрессированных дат
            std::set< std::string > allCurCandleDates;
            {
                auto resVec = pDbMan->queryToSingleStringVector( pDbMan->execHelper( selectCandleDateTimeFromInstrumentCandlesQueryText ) // результат выполнения игнорим
                                                               , 0 // индекс требуемой величины в векторо строки результата
                                                               );

                if (!resVec.empty()) // особого смысла не имеет, просто, чтобы брякнуться в отладчик, если множество не пустое
                {
                    transform( resVec.begin(), resVec.end()
                             , std::inserter(allCurCandleDates, allCurCandleDates.begin())
                             , []( const QString &dtStr )
                                 {
                                     return qt_helpers::compressDateTimeString<std::string,QString>(dtStr);
                                 }
                             );
                }

            }
            //selectCandleDateTimeFromInstrumentCandlesQueryText


            // Ду Жоб Хере

            cout << "      " << "Starting date: " << dtLastCandleDate << endl;


            QDateTime curDateTime = QDateTime::currentDateTime().toUTC();

            std::vector< QDateTime > candleIntervals;

            QDateTime dateTime;

            if (candleResolutionId<=8)
            {
                cout << "      " << "Intraday candle interval detected" << endl;
                // Свечи внутри дня

                // Нужно захватить конец дня

                dateTime = dtLastCandleDate;
                QDateTime dateTimeDayStart = qt_helpers::makeMidnightDateTime(dateTime);
                if (dateTimeDayStart==dateTime)
                {
                    // Дата время совпадает с началом дня - ничего не делаем
                    cout << "      " << "Starting date-time equals to day start" << endl;
                    candleIntervals.push_back(dateTime);
                }
                else
                {
                    // Кладём стартовую дату-время, обнуляем время, и прибавляем интервал - он будет стартовой датой для дальнейшего алгоритма
                    // Таким образом, добавили хвостик от текущего дня до начала следующего
                    cout << "      " << "Starting date-time NOT equals to day start, starting date-time: " << dateTime << endl;
                    candleIntervals.push_back(dateTime);
                    dateTime = qt_helpers::dtAddTimeInterval(dateTimeDayStart, candleIntervalRec);
                    if (dateTime == curDateTime)
                        dateTime = curDateTime;
                    cout << "      " << "                                    starting day end date-time: " << dateTime << endl;
                    candleIntervals.push_back(dateTime);
                }

            }
            else
            {
                dateTime = qt_helpers::makeMidnightDateTime(dtLastCandleDate);
                candleIntervals.push_back(dateTime);
            }


            while(true)
            {
                QDateTime nextDateTime = qt_helpers::dtAddTimeInterval(dateTime, candleIntervalRec);

                //cout << "      " << "Next date    : " << nextDateTime << endl;

                if (nextDateTime>curDateTime)
                    nextDateTime = curDateTime;

                if (candleIntervals.empty())
                {
                    candleIntervals.push_back(nextDateTime);
                }
                else
                {
                    if (nextDateTime > candleIntervals.back())
                    {
                        candleIntervals.push_back(nextDateTime);
                    }
                    else
                    {
                        break;
                    }
                }

                dateTime = nextDateTime;

            }

            cout << "------------------------------" << endl;
            cout << "!!! Calculated start date&times for " << candleResolution << " candles" << endl;
            for( std::vector< QDateTime >::const_iterator dtIt = candleIntervals.begin(); dtIt != candleIntervals.end(); ++dtIt )
            {
                cout << "        " << *dtIt << endl;
            }
            cout << endl;


            /*
            std::vector< QDateTime >::const_iterator intervalIt    = candleIntervals.begin();
            std::vector< QDateTime >::const_iterator intervalItEnd = candleIntervals.end();

            if (intervalIt==intervalItEnd)
            {
                continue;
            }

            std::vector< QDateTime >::const_iterator curIntervalItBegin = intervalIt;
            ++intervalIt;
            std::vector< QDateTime >::const_iterator curIntervalItEnd   = intervalIt;
            */

            if (candleIntervals.size()<2)
            {
                cout << "/// Number of candle dates less from 2" << endl;
                continue;
            }

            std::vector< QDateTime >::size_type candleIntervalStartIdx = 0;
            std::vector< QDateTime >::size_type candleIntervalEndIdx   = 1;

            unsigned numItemsInPrevCandleResponse = 0;

            singleRequestTimer.restart();

            for( ; candleIntervalEndIdx!=candleIntervals.size(); ++candleIntervalStartIdx, ++candleIntervalEndIdx )
            {
                //cout << "    " << *curIntervalItBegin << " - " << *curIntervalItEnd << endl;

                singleRequestElapsedTime = (unsigned)singleRequestTimer.restart();

                QDateTime candleIntervalStartDateTime = candleIntervals[candleIntervalStartIdx];
                QDateTime candleIntervalEndDateTime   = candleIntervals[candleIntervalEndIdx];

                cout << "------------------------------" << endl
                     << "!!! Candles for instrument " << instrumentId << ", FIGI: " << figi << " (" << ticker << ") - " << instrumentName << endl
                     << "    Resolution: " << candleResolution 
                     << ", interval: " << candleIntervalMax << endl
                     << "    Start date: " << candleIntervalStartDateTime << endl
                     << "    End   date: " << candleIntervalEndDateTime << endl
                     
                     << endl
                     << "    Prev request completed at ??? : " << singleRequestElapsedTime << " ms" << endl
                     << "    Total candle items in response: " << numItemsInPrevCandleResponse
                     << endl
                     ;

                if ( candleIntervalStartDateTime.offsetFromUtc() != 0 )
                {
                    cout << "*** Abort: interval start date&time is not UTC" << endl;
                    return 1;
                }

                if ( candleIntervalEndDateTime.offsetFromUtc() != 0 )
                {
                    cout << "*** Abort: interval end date&time is not UTC" << endl;
                    return 1;
                }


                // Разрешено не более 120 запросов в минуту
                const unsigned defaultDelay = 60*1000 / 120 + 50; // Подождать чуть дольше - не жалко
                // Всё равно на порядок больше выиграли, когда стали добавлять дынные пачками

                unsigned waitTime = 0;

                if (singleRequestElapsedTime<defaultDelay)
                {
                    unsigned delayTime = defaultDelay - singleRequestElapsedTime;
                    cout << "    Waiting for " << delayTime << " ms" << endl;
                    QTest::qWait(delayTime);
                }

                // cout << "Retrieving data from server" << endl;

                #if defined(DEBUG) || defined(_DEBUG)

                QString candleIntervalStartDateStr = qt_helpers::dateTimeToDbString(candleIntervalStartDateTime);
                QString candleIntervalEndDateStr   = qt_helpers::dateTimeToDbString(candleIntervalEndDateTime);

                do
                {
                } while(0);
               
                #endif


                auto // CandlesResponse
                //candlesRes = pOpenApi->marketCandles( figi, *curIntervalItBegin, *curIntervalItEnd, candleResolution );
                //candlesRes = pOpenApi->marketCandles( figi, candleIntervals[candleIntervalStartIdx], candleIntervals[candleIntervalEndIdx], candleResolution );
                candlesRes = pOpenApi->marketCandles( figi, candleIntervalStartDateTime, candleIntervalEndDateTime, candleResolution );
                
               
                // candlesRes->join();
                //  
                // if (candlesRes->isCompletionError())

                if (candlesRes->join().isCompletionError())
                {
                    cout << endl
                         << "*** Error: (HTTP/Server error) failed to get " << candleResolution 
                         << " candles for " << figi << " (" << ticker << ") on interval: " 
                         << candleIntervalStartDateTime << " - " << candleIntervalEndDateTime << endl
                         << "    Error message: " << candlesRes->getErrorMessage() << endl
                         ;

                    bool erroneous = false;

                    // Error message: Message: Error transferring https://api-invest.tinkoff.ru/openapi/market/candles?figi=BBG00GXQKMJ9&from=2020-01-01T00%3A00%3A00.000000
                    // {"trackingId":"f08b8b3c4f9ec987","payload":{"message":"[figi]: Instrument not found by figi=BBG00GXQKMJ9","code":"VALIDATION_ERROR"},"status":"Error"}B00%3A00&to=2021-05-20T16%3A14%3A35.455000{"trackingId":"f08b8b3c4f9ec987","payload":{"message":"[figi]: Instrument not found by figi=BBG00GXQKMJ9","code":"VALIDATION_ERROR"},"status":"Error"}B00%3A00&interval=month - server replied: Internal Server Error, {"trackingId":"f08b8b3c4f9ec987","payload":{"message":"[figi]: Instrument not found by figi=BBG00GXQKMJ9","code":"VALIDATION_ERROR"},"status":"Error"}
                    // Error with this figi

                    static std::vector<QString> errCheckList1 = std::vector<QString>{ QString("Instrument not found by figi")
                                                                                    , QString("\"code\":\"VALIDATION_ERROR\"")
                                                                                    , QString("\"status\":\"Error\"")
                                                                                    };
                    static std::vector<QString> errCheckList2 = std::vector<QString>{ QString("Status: Error")
                                                                                    , QString("Code: VALIDATION_ERROR")
                                                                                    , QString("Instrument not found by figi")
                                                                                    };

                    if ( candlesRes->checkErrorMessageFor( Qt::CaseInsensitive, errCheckList1 )
                      || candlesRes->checkErrorMessageFor( Qt::CaseInsensitive, errCheckList2 )
                       )
                    {
                        // erroneousFigis.insert(figi);
                        erroneous = true;
                        cout << "*** Error detected by checkErrorMessageFor" << endl;
                    }

                    OpenAPI::Error oaErr = candlesRes->getErrorAsObject();
                    QString oaErrStatus  = oaErr.getStatus();

                    OpenAPI::Error_payload oaErrData = oaErr.getPayload();
                    QString oaErrCode = oaErrData.getCode();

                    if (oaErrStatus.compare(QString("Error"), Qt::CaseInsensitive)==0 && oaErrCode.compare(QString("VALIDATION_ERROR"), Qt::CaseInsensitive)==0)
                    {
                        // erroneousFigis.insert(figi);
                        erroneous = true;
                        cout << "*** Error detected by ErrStatus and ErrCode" << endl;
                    }

                    cout << endl;

                    if (erroneous)
                    {
                        erroneousFigis.insert(figi);
                    }

                    continue;


                    // [figi]: Instrument not found by figi
                    // "code":"VALIDATION_ERROR"
                    // "status":"Error"

                    // OpenAPI::Error getErrorAsObject() const
                    // QString getStatus()
                    // Error_payload getPayload()
                    //    QString getMessage() const;
                    //    QString getCode() const;

                }

                QElapsedTimer dbInsertionsTimer;
                dbInsertionsTimer.start();

                //std::vector<tkf::Candle>
                auto candles = tkf::makeVectorFromList(candlesRes->value.getPayload().getCandles());

                if (!candles.empty())
                    atLeastOneCandleFound = true;

                if ( candleResolutionId<=8 && !atLeastOneCandleFound )
                {
                    // Нет свечей часовых или на более короткие интервалы на данную дату
                    // Логично предположить, что и более коротких свечей тоже нет.
                    // Поэтому для более коротких свечей смещаем стартовую дату до текущей

                    QDate startDate = candleIntervalStartDateTime.date();

                    for( int idx=candleResolutionId; idx!=0; --idx )
                    {
                        candleIntervalStartDates[idx] = startDate;
                    }

                }

                QVector< QVector<QString> > candleDataForInsertion;

                for( const auto &candle : candles )
                {
                    if (!candle.isSet())
                    {
                        cout << endl
                             << "*** Error: Candle not set" 
                             << endl;
                        continue;
                    }

                    if (!candle.isValid())
                    {
                        cout << endl
                             << "*** Error: Candle not is not valid" 
                             << endl;
                        continue;
                    }

                    auto candleFigi = candle.getFigi();
                    if (candleFigi!=figi)
                    {
                        cout << endl
                             << "*** Error: Candle FIGI (" << candleFigi << " not equal to current instrument FIGI (" << figi << ")"
                             << endl;
                        continue;
                    }

                    // Почему-то иногда вылетает добавление в базу по юник констрайнту
                    // Добавлять мы хотим пачкой - это на порядок быстрее, поэтому надо убедится, что такой даты в базе 
                    // для данного свечного интервала, инструмента и биржи - нет

                    // std::set< std::string > allCurCandleDates;
                    auto candleDt = candle.getTime();
                    QString candleDtStr = qt_helpers::dateTimeToDbString( candleDt );
                    std::string compressedCandleDtStr = qt_helpers::compressDateTimeString<std::string,QString>(candleDtStr);

                    // Table: INSTRUMENT_CANDLES
                    //auto instrumentCandlesTableFields = QString("INSTRUMENT_ID,STOCK_EXCHANGE_ID,CANDLE_RESOLUTION_ID,CANDLE_DATE_TIME,CURRENCY_ID,OPEN_PRICE,CLOSE_PRICE,HIGH_PRICE,LOW_PRICE,VOLUME");
                    auto valuesToInsert = QString("%1,%2,%3,%4,%5,%6,%7,%8,%9,%10")
                                                 .arg(instrumentId)         // INSTRUMENT_ID
                                                 .arg(stockExchangeId)      // STOCK_EXCHANGE_ID
                                                 .arg(candleResolutionId)   // CANDLE_RESOLUTION_ID
                                                 .arg(candleDtStr)          // CANDLE_DATE_TIME
                                                 .arg(instrumentCurrencyId) // CURRENCY_ID
                                                 .arg(QString::fromStdString(candle.getO().toString())) // OPEN_PRICE 
                                                 .arg(QString::fromStdString(candle.getC().toString())) // CLOSE_PRICE
                                                 .arg(QString::fromStdString(candle.getH().toString())) // HIGH_PRICE 
                                                 .arg(QString::fromStdString(candle.getL().toString())) // LOW_PRICE  
                                                 .arg(QString::number(candle.getV()))                   // VOLUME     
                                                 ;

                    // cout << "Inserting data to DB" << endl;

                    if (allCurCandleDates.find(compressedCandleDtStr) != allCurCandleDates.end())
                    {
                         cout
                         << "    Candles for " << figi << " (" << ticker << "), resolution: " << candleResolution 
                         << " started at : " 
                         << candleDtStr /* candleIntervalStartDateTime */  << " - already exist in DB" << endl;

                         // QStringList fieldsLstTmp    = instrumentCandlesTableFields.split(',');
                         // QStringList whereFieldsList = QStringList( fieldsLstTmp.begin(), std::advance(fieldsLstTmp.begin(), 4) );

                         // QStringList valuesLstTmp    = valuesToInsert.split(',');
                         // QStringList whereValuesList = QStringList( valuesLstTmp.begin(), std::advance(valuesLstTmp.begin(), 4) );

                         QStringList whereFieldsList = tkf::getPartialStringList( instrumentCandlesTableFields.split(',', Qt::KeepEmptyParts), 4 );
                         QStringList whereValuesList = tkf::getPartialStringList( valuesToInsert              .split(',', Qt::KeepEmptyParts), 4 );

                         QString whereFieldsStr = tkf::mergeString(whereFieldsList, ",");
                         QString whereValuesStr = tkf::mergeString(whereValuesList, ",");
                         

                         QString updateQuery = pDbMan->makeSimpleUpdateQueryText( "INSTRUMENT_CANDLES"
                                                                                , whereFieldsStr // whereFields
                                                                                , whereValuesStr // whereVal
                                                                                , tkf::convertToQVectorOfQStrings(valuesToInsert) // valuesToInsert
                                                                                , instrumentCandlesTableFields // whereName
                                                                                );

                        pDbMan->execHelper( updateQuery );

                        continue;
                    }

                    allCurCandleDates.insert(compressedCandleDtStr);


                    candleDataForInsertion.push_back( tkf::convertToQVectorOfQStrings(valuesToInsert) );
                    /*
                    //QVector<QString> valsVec = {valuesToInsert};
                    pDbMan->insertTo( "INSTRUMENT_CANDLES"
                                    , QVector< QVector<QString> >{ tkf::convertToQVectorOfQStrings(valuesToInsert) }
                                    , instrumentCandlesTableFields
                                    );
                    */

                } // for( const auto &candle : candles )

                numItemsInPrevCandleResponse = candleDataForInsertion.size();

                if (!candleDataForInsertion.empty())
                {
                    pDbMan->insertTo( "INSTRUMENT_CANDLES"
                                    , candleDataForInsertion
                                    , instrumentCandlesTableFields
                                    );
                }

                cout << endl
                     << "Insertions time summary: " << (unsigned)dbInsertionsTimer.elapsed()
                     << endl
                     << endl
                     ;

            /*
            tableSchemas[QString("INSTRUMENT_CANDLES" )] = "INSTRUMENT_ID         INTEGER REFERENCES MARKET_INSTRUMENT,"   + lf() +
                                                           "STOCK_EXCHANGE_ID     INTEGER REFERENCES STOCK_EXCHANGE_LIST," + lf() +
                                                           "CANDLE_RESOLUTION_ID  INTEGER REFERENCES CANDLE_RESOLUTION,"   + lf() +
                                                           "CANDLE_DATE_TIME      VARCHAR(24) NOT NULL,"                   + lf() +
                                                           "CURRENCY_ID           INTEGER REFERENCES CURRENCY,"            + lf() +
                                                           "OPEN_PRICE            DECIMAL(18,8) NOT NULL,"                 + lf() +
                                                           "CLOSE_PRICE           DECIMAL(18,8) NOT NULL,"                 + lf() +
                                                           "HIGH_PRICE            DECIMAL(18,8) NOT NULL,"                 + lf() +
                                                           "LOW_PRICE             DECIMAL(18,8) NOT NULL,"                 + lf() +
                                                           "VOLUME                DECIMAL(18,8) NOT NULL"
                                                         ;
            */    

            }


            // qt_helpers::makeMidnightDateTime( const QDate &date )
            // qt_helpers::makeMidnightDateTime( const QDateTime &dt )
            // QString candleIntervalMax = dicts.getCandleResolutionIntervalMaxById(candleResolutionId);


            //QDateTime qt_helpers::dtAddTimeInterval( const QDateTime &dt, QString intervalStr, int forceSign = 0 /* 0 - use default */ )


            cout
                 << endl
                 ;


        } // for(; candleResolutionId!=candleResolutionIdEnd && !ctrlC.isBreaked(); ++candleResolutionId)

        auto figiElapsedTime = instrumentCandlesTimer.elapsed();
        cout << "Get instrument " << figi << " (" << ticker << ") candles completed in " << (unsigned)figiElapsedTime << "ms" << endl;

        //std::map<QString, std::uint64_t> 
        figiTimes[figi] = figiElapsedTime;

    } // for( ; instrumentId!=instrumentIdEnd && !ctrlC.isBreaked(); ++instrumentId )


    cout << endl << endl << "-----------------"
         << endl << " Instruments update statistic:" << endl;

    std::set<QString> allBadFigis = tkf::merge(discontinuedFigis, erroneousFigis);

    for( auto badFigi : allBadFigis )
    {

        QString ticker         = dicts.getTickerByFigiChecked(badFigi);
        QString instrumentName = dicts.getNameByFigiChecked(badFigi);

        if (discontinuedFigis.find(badFigi)!=discontinuedFigis.end())
        {
            cout << "FIGI: " << badFigi << " is delisted. Ticker: " << ticker << ", name - " << instrumentName << endl;
            continue;
        }

        if (erroneousFigis.find(badFigi)!=erroneousFigis.end())
        {
            cout << "FIGI: " << badFigi << " is erroneous. Ticker: " << ticker << ", name - " << instrumentName << endl;
            continue;
        }

        cout << "FIGI: " << badFigi << " is blocked by unknown reason"  << endl;

    } // for( ; instrumentId!=instrumentIdEnd && !ctrlC.isBreaked(); ++instrumentId )



    struct FigiTimes
    {
        QString              figi;
        std::uint64_t        totalCandlesTime;
    };

    auto figiCandleTimeLess = []( const FigiTimes &ft1, const FigiTimes &ft2 )
    {
        return ft1.totalCandlesTime < ft2.totalCandlesTime;
    };

    auto figiCandleTimeGreater = []( const FigiTimes &ft1, const FigiTimes &ft2 )
    {
        return ft1.totalCandlesTime > ft2.totalCandlesTime;
    };


    std::vector<FigiTimes> sortedFigiTimeByCandleGetTime;

    std::map<QString, std::uint64_t>::const_iterator figiCandleTimeIt = figiTimes.begin();
    for(; figiCandleTimeIt!=figiTimes.end(); ++figiCandleTimeIt)
    {
        sortedFigiTimeByCandleGetTime.push_back( FigiTimes{ figiCandleTimeIt->first, figiCandleTimeIt->second } );
    }

    std::sort( sortedFigiTimeByCandleGetTime.begin(), sortedFigiTimeByCandleGetTime.end(), figiCandleTimeLess );


    std::vector<FigiTimes>::const_iterator ftIt = sortedFigiTimeByCandleGetTime.begin();
    for(; ftIt != sortedFigiTimeByCandleGetTime.end(); ++ftIt)
    {
        QString figi = ftIt->figi;
        std::uint64_t et = ftIt->totalCandlesTime;

        QString ticker         = dicts.getTickerByFigiChecked(figi);
        QString instrumentName = dicts.getNameByFigiChecked(figi);

        cout << figi << ": " << (unsigned)et << ", " << ticker << " - " << instrumentName << endl;
    
    }


    /*
    instrumentId     = dicts.getInstrumentIdBegin( );
    //instrumentIdEnd  = dicts.getInstrumentIdEnd( );

    for( ; instrumentId!=instrumentIdEnd && !ctrlC.isBreaked(); ++instrumentId )
    {
        if (!dicts.isValidId(instrumentId))
            continue;

        QString figi = dicts.getInstrumentById( instrumentId );
        if (figi.isEmpty())
            continue; // There is a GAP found in instruments enumeration
    }
    */
/*

    std::set<QString> discontinuedFigis;
    std::set<QString> erroneousFigis;
    std::map<QString, std::uint64_t> figiTimes;

*/
    
    return 0;
}



