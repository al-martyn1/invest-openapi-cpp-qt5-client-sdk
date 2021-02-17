/*! \file
    \brief 
 */

#pragma once

#include <QObject>
#include <QDebug>
#include <QtDebug>
#include <QTest>
#include <QTimeZone>

//#include <QtConcurrent/QtConcurrent>

#include <iostream>
#include <exception>
#include <stdexcept>

#include "models.h"

#include "invest_openapi_safe_main.h"
#include "openapi_completable_future_base.h"
#include "openapi_completable_future.h"
#include "api_config.h"
#include "auth_config.h"
#include "logging_config.h"
#include "currencies_config.h"
#include "factory.h"
#include "utility.h"
#include "qdebug_support.h"
#include "marty_decimal.h"
#include "qt_time_helpers.h"



//----------------------------------------------------------------------------

#if defined(_MSC_VER)
    #pragma comment(lib, "tkf_invest_oa")
#endif

//----------------------------------------------------------------------------

// Invest OpenAPI interface method
// #define IOAIM

#define TKF_IOA_ABSTRACT_METHOD_END   = 0
#define TKF_IOA_METHOD_IMPL_END       override

#define TKF_IOA_SHARED_COMPLETABLE_FUTURE( T )               QSharedPointer< OpenApiCompletableFuture< T > >
#define TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( T, valName )  \
        TKF_IOA_SHARED_COMPLETABLE_FUTURE( T ) valName = TKF_IOA_SHARED_COMPLETABLE_FUTURE( T )( new OpenApiCompletableFuture< T >() )


#define TKF_IOA_ABSTRACT_METHOD( ret, nameWithParams )     virtual TKF_IOA_SHARED_COMPLETABLE_FUTURE( ret ) nameWithParams TKF_IOA_ABSTRACT_METHOD_END
#define TKF_IOA_METHOD_IMPL( ret, nameWithParams )         virtual TKF_IOA_SHARED_COMPLETABLE_FUTURE( ret ) nameWithParams TKF_IOA_METHOD_IMPL_END

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
namespace invest_openapi
{


using marty::DecimalPrecision;
using marty::Decimal         ;
using marty::toString        ;
using marty::fromString      ;



//----------------------------------------------------------------------------
inline
void pollMessageQueue()
{
    QTest::qWait(0);
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
class OpenApiFactory;




//----------------------------------------------------------------------------
struct IOpenApi
{
    virtual void    setBrokerAccountId( const QString &id ) = 0;
    virtual QString getBrokerAccountId()                    = 0;

    virtual void setRequestsDebug( bool v )  = 0;
    virtual void setResponsesDebug( bool v ) = 0;
    virtual bool getRequestsDebug( )         = 0;
    virtual bool getResponsesDebug( )        = 0;


    //------------------------------
    // MarketApi

    TKF_IOA_ABSTRACT_METHOD( MarketInstrumentListResponse, marketBonds() );
    TKF_IOA_ABSTRACT_METHOD( MarketInstrumentListResponse, marketCurrencies() );
    TKF_IOA_ABSTRACT_METHOD( MarketInstrumentListResponse, marketEtfs() );
    TKF_IOA_ABSTRACT_METHOD( MarketInstrumentListResponse, marketStocks() );

    TKF_IOA_ABSTRACT_METHOD( MarketInstrumentListResponse, marketInstruments( const InstrumentType &instrumentType ) );

    /*
    // Не работает без Паблика Морозова, поэтому лучше по отдельности будем дергать
    TKF_IOA_ABSTRACT_METHOD( MarketInstrumentListResponse, marketInstruments( InstrumentType::eInstrumentType instrumentType ) );
    TKF_IOA_ABSTRACT_METHOD( MarketInstrumentListResponse, marketInstruments( const QString &instrumentType ) );
    TKF_IOA_ABSTRACT_METHOD( MarketInstrumentListResponse, marketInstruments( ) ); // All instruments
    */

    // Границы интервала дат для получения данных по свечам см. doc/candle_intervals.png
    /*
    Параметры свечи (Candle)
    o     double + Цена открытия
    c     double + Цена закрытия
    h     double + Наибольшая цена
    l     double + Наименьшая цена
    v     double + Объем торгов
    time  string + Время в формате RFC3339
    */

    TKF_IOA_ABSTRACT_METHOD( CandlesResponse, marketCandles( const QString &figi, const QDateTime &from, const QDateTime &to, const CandleResolution &interval ) );
    TKF_IOA_ABSTRACT_METHOD( CandlesResponse, marketCandles( const QString &figi, const QDateTime &from, const QDateTime &to, CandleResolution::eCandleResolution interval ) );
    TKF_IOA_ABSTRACT_METHOD( CandlesResponse, marketCandles( const QString &figi, const QDateTime &from, const QDateTime &to, const QString &interval ) );

    // Стакан по фиге, глубина макс 20
    TKF_IOA_ABSTRACT_METHOD( OrderbookResponse, marketOrderbook( const QString &figi, qint32 depth ) );
    // Выбираем стакан с максимальной глубиной
    TKF_IOA_ABSTRACT_METHOD( OrderbookResponse, marketOrderbook( const QString &figi ) );
    

    //------------------------------
    // UserApi

    TKF_IOA_ABSTRACT_METHOD( UserAccountsResponse, userAccounts( ) );
    

    //------------------------------
    // PortfolioApi

    TKF_IOA_ABSTRACT_METHOD( PortfolioCurrenciesResponse, portfolioCurrencies(QString broker_account_id = QString()) );
    TKF_IOA_ABSTRACT_METHOD( PortfolioResponse, portfolio(QString broker_account_id = QString()) );


    //------------------------------
    // OrdersApi

    TKF_IOA_ABSTRACT_METHOD( LimitOrderResponse , ordersLimitOrder (const QString &figi, const LimitOrderRequest  &limit_order_request , QString broker_account_id = QString()) );
    TKF_IOA_ABSTRACT_METHOD( LimitOrderResponse , ordersLimitOrder (const QString &figi, const OperationType &operation, qint32 nLots  , double price, QString broker_account_id = QString()) );
    TKF_IOA_ABSTRACT_METHOD( LimitOrderResponse , ordersLimitOrder (const QString &figi, const OperationType::eOperationType &operation, qint32 nLots, double price, QString broker_account_id = QString()) );
    TKF_IOA_ABSTRACT_METHOD( LimitOrderResponse , ordersLimitOrder (const QString &figi, const QString &operation      , qint32 nLots  , double price, QString broker_account_id = QString()) );

    TKF_IOA_ABSTRACT_METHOD( MarketOrderResponse, ordersMarketOrder(const QString &figi, const MarketOrderRequest &market_order_request, QString broker_account_id = QString()) );
    TKF_IOA_ABSTRACT_METHOD( MarketOrderResponse, ordersMarketOrder(const QString &figi, const OperationType &operation, qint32 nLots  , QString broker_account_id = QString()) );
    TKF_IOA_ABSTRACT_METHOD( MarketOrderResponse, ordersMarketOrder(const QString &figi, const OperationType::eOperationType &operation, qint32 nLots, QString broker_account_id = QString()) );
    TKF_IOA_ABSTRACT_METHOD( MarketOrderResponse, ordersMarketOrder(const QString &figi, const QString &operation      , qint32 nLots  , QString broker_account_id = QString()) );

    TKF_IOA_ABSTRACT_METHOD( Empty, ordersCancel(const QString &order_id, QString broker_account_id = QString() ));
    TKF_IOA_ABSTRACT_METHOD( OrdersResponse, orders(QString broker_account_id = QString() ));

    //------------------------------
    // OperationsApi

    TKF_IOA_ABSTRACT_METHOD( OperationsResponse , operations ( const QDateTime &from, const QDateTime &to, const QString &figi, QString broker_account_id = QString()) );

/*
    void operationsGet(const QDateTime &from, const QDateTime &to, const QString &figi, const QString &broker_account_id);
    void operationsGetSignal(OperationsResponse summary);

*/

    //------------------------------
    // Helpers
    virtual bool findInstrumentListingStartDate( const QString &figi, const QDate &dateStartLookupFrom, QDate &foundDate ) = 0;


    virtual ~IOpenApi() {};
}; // struct IOpenApi

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
struct ISanboxOpenApi // : public IOpenApi
{
    virtual void    setBrokerAccountId( const QString &id ) = 0;
    virtual QString getBrokerAccountId()                    = 0;

    TKF_IOA_ABSTRACT_METHOD( SandboxRegisterResponse, sandboxRegister(BrokerAccountType v) );
    TKF_IOA_ABSTRACT_METHOD( SandboxRegisterResponse, sandboxRegister(BrokerAccountType::eBrokerAccountType v) );

    TKF_IOA_ABSTRACT_METHOD( Empty, sandboxClear(QString broker_account_id = QString() ) );

    TKF_IOA_ABSTRACT_METHOD( Empty, sandboxCurrenciesBalanceSet(const SandboxCurrency::eSandboxCurrency currency, Decimal balance, QString broker_account_id = QString() ) );
    TKF_IOA_ABSTRACT_METHOD( Empty, sandboxCurrenciesBalanceSet(const SandboxCurrency                  &currency, Decimal balance, QString broker_account_id = QString() ) );
    TKF_IOA_ABSTRACT_METHOD( Empty, sandboxCurrenciesBalanceSet(const QVector<CurrencyConfig>           currencies, QString broker_account_id = QString() ) );

    TKF_IOA_ABSTRACT_METHOD( Empty, sandboxPositionsBalanceSet(const SandboxSetPositionBalanceRequest          &positionBalance , QString broker_account_id = QString() ) );
    TKF_IOA_ABSTRACT_METHOD( Empty, sandboxPositionsBalanceSet(const QString &positionFigi, const marty::Decimal &positionBalance, QString broker_account_id = QString() ) );
    TKF_IOA_ABSTRACT_METHOD( Empty, sandboxPositionsBalanceSet(const QVector<SandboxSetPositionBalanceRequest> &positionBalances, QString broker_account_id = QString() ) );
    
    TKF_IOA_ABSTRACT_METHOD( Empty, sandboxRemove(QString broker_account_id = QString()) );

/*
    TKF_IOA_ABSTRACT_METHOD( Empty, () );
*/
}; // struct ISanboxOpenApi

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
class OpenApiImpl : public IOpenApi
{

    friend class OpenApiFactory;

    //------------------------------

protected:

    void checkBrokerAccountIdParam( QString &id )
    {
        if (id.isEmpty())
        {
            if (m_brokerAccountId.isEmpty())
                throw std::runtime_error("Broker account ID not set");
            id = m_brokerAccountId;
        }
    }

    //------------------------------


public:

    //------------------------------
    OpenApiImpl( const ApiConfig  &apiConfig
               , const AuthConfig &authConfig
               , const LoggingConfig &loggingConfig
               )
    : m_apiConfig(apiConfig)
    , m_authConfig(authConfig)
    , m_loggingConfig(loggingConfig)
    {
        initApis(OpenApiFactory(m_apiConfig, m_authConfig));

        setRequestsDebug (m_loggingConfig.debugRequests);
        setResponsesDebug(m_loggingConfig.debugResponses);
    }

    //------------------------------
    virtual void    setBrokerAccountId( const QString &id ) override
    {
        m_brokerAccountId = id;
    }

    //------------------------------
    virtual QString getBrokerAccountId()                    override
    {
        return m_brokerAccountId;
    }

    //------------------------------
    virtual void setRequestsDebug( bool v )  override
    {
        ::OpenAPI::setRequestsDebug(v); // prevent self-calling by using full qualified name
    }

    virtual bool getRequestsDebug( )         override
    {
        return ::OpenAPI::getRequestsDebug(); // prevent self-calling by using full qualified name
    }

    virtual void setResponsesDebug( bool v ) override
    {
        ::OpenAPI::setResponsesDebug(v); // prevent self-calling by using full qualified name
    }

    virtual bool getResponsesDebug( )        override
    {
        return ::OpenAPI::getResponsesDebug(); // prevent self-calling by using full qualified name
    }

    //------------------------------



    
    //------------------------------
    // MarketApi
    //------------------------------

    
    //------------------------------
    TKF_IOA_METHOD_IMPL( MarketInstrumentListResponse, marketBonds() )
    {
        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( MarketInstrumentListResponse, response );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( response.get(), m_pMarketApi.get(), marketBonds, Get );
        m_pMarketApi->marketBondsGet();

        return response;
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( MarketInstrumentListResponse, marketCurrencies() )
    {
        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( MarketInstrumentListResponse, response );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( response.get(), m_pMarketApi.get(), marketCurrencies, Get );
        m_pMarketApi->marketCurrenciesGet();

        return response;
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( MarketInstrumentListResponse, marketEtfs() )
    {
        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( MarketInstrumentListResponse, response );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( response.get(), m_pMarketApi.get(), marketEtfs, Get );
        m_pMarketApi->marketEtfsGet();

        return response;
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( MarketInstrumentListResponse, marketStocks() )
    {
        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( MarketInstrumentListResponse, response );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( response.get(), m_pMarketApi.get(), marketStocks, Get );
        m_pMarketApi->marketStocksGet();

        return response;
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( MarketInstrumentListResponse, marketInstruments( const InstrumentType &instrumentType ) )
    {
        switch(instrumentType.getValue())
        {
            case InstrumentType::eInstrumentType::STOCK   : return marketStocks();
            case InstrumentType::eInstrumentType::CURRENCY: return marketCurrencies();
            case InstrumentType::eInstrumentType::BOND    : return marketBonds();
            case InstrumentType::eInstrumentType::ETF     : return marketEtfs();
        }

        throw std::runtime_error("marketInstruments: Invalid instrument type");
        return marketStocks();
    }

    /*
    //------------------------------
    TKF_IOA_METHOD_IMPL( MarketInstrumentListResponse, marketInstruments( InstrumentType::eInstrumentType instrumentType ) )
    {
        return marketInstruments( toInstrumentType(instrumentType) );
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( MarketInstrumentListResponse, marketInstruments( const QString &instrumentType ) )
    {
        return marketInstruments( toInstrumentType(instrumentType) );
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( MarketInstrumentListResponse, marketInstruments( ) ) // All instruments
    {
        auto resStocks     = marketStocks();
        auto resCurrencies = marketCurrencies();
        auto resBonds      = marketBonds();
        auto resEtfs       = marketEtfs();

        resStocks     ->join();
        resCurrencies ->join();
        resBonds      ->join();
        resEtfs       ->join();

        if (resStocks     ->isCompletionError()) return resStocks     ;
        if (resCurrencies ->isCompletionError()) return resCurrencies ;
        if (resBonds      ->isCompletionError()) return resBonds      ;
        if (resEtfs       ->isCompletionError()) return resEtfs       ;

        mergeResponse(resStocks->value, resCurrencies->value);
        mergeResponse(resStocks->value, resBonds     ->value);
        mergeResponse(resStocks->value, resEtfs      ->value);

        return resStocks;
    }
    */

    //------------------------------
    TKF_IOA_METHOD_IMPL( CandlesResponse, marketCandles( const QString &figi, const QDateTime &from, const QDateTime &to, const CandleResolution &interval ) )
    {
        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( CandlesResponse, response );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( response.get(), m_pMarketApi.get(), marketCandles, Get );
        m_pMarketApi->marketCandlesGet(figi, from, to, interval);
        return response;
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( CandlesResponse, marketCandles( const QString &figi, const QDateTime &from, const QDateTime &to, CandleResolution::eCandleResolution interval ) )
    {
        return marketCandles(figi, from, to, toCandleResolution(interval));
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( CandlesResponse, marketCandles( const QString &figi, const QDateTime &from, const QDateTime &to, const QString &interval ) )
    {
        return marketCandles(figi, from, to, toCandleResolution(interval));
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( OrderbookResponse, marketOrderbook( const QString &figi, qint32 depth ) )
    {
        if (depth>20)
            depth = 20;
        if (depth<3)
            depth = 3;

        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( OrderbookResponse, response );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( response.get(), m_pMarketApi.get(), marketOrderbook, Get );
        m_pMarketApi->marketOrderbookGet(figi, depth);
        return response;
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( OrderbookResponse, marketOrderbook( const QString &figi ) )
    {
        return marketOrderbook(figi, 20);
    }

    //------------------------------


    //------------------------------
    // UserApi
    //------------------------------


    //------------------------------
    TKF_IOA_METHOD_IMPL( UserAccountsResponse, userAccounts( ) )
    {
        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( UserAccountsResponse, response );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( response.get(), m_pUserApi.get(), userAccounts, Get );
        m_pUserApi->userAccountsGet();
        return response;
    }

    //------------------------------


    //------------------------------
    // PortfolioApi
    //------------------------------


    //------------------------------
    TKF_IOA_METHOD_IMPL( PortfolioCurrenciesResponse, portfolioCurrencies(QString broker_account_id = QString()) )
    {
        checkBrokerAccountIdParam(broker_account_id);

        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( PortfolioCurrenciesResponse, response );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( response.get(), m_pPortfolioApi.get(), portfolioCurrencies, Get );
        m_pPortfolioApi->portfolioCurrenciesGet(broker_account_id);
        return response;
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( PortfolioResponse, portfolio(QString broker_account_id = QString()) )
    {
        checkBrokerAccountIdParam(broker_account_id);

        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( PortfolioResponse, response );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( response.get(), m_pPortfolioApi.get(), portfolio, Get );
        m_pPortfolioApi->portfolioGet(broker_account_id);
        return response;
    }

    //------------------------------

    
    //------------------------------
    // OrdersApi
    //------------------------------


    //------------------------------
    TKF_IOA_METHOD_IMPL( LimitOrderResponse , ordersLimitOrder(const QString &figi, const LimitOrderRequest &limit_order_request  , QString broker_account_id = QString()) )
    {
        checkBrokerAccountIdParam(broker_account_id);

        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( LimitOrderResponse, response );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( response.get(), m_pOrdersApi.get(), ordersLimitOrder, Post );
        m_pOrdersApi->ordersLimitOrderPost(figi, limit_order_request, broker_account_id);
        return response;
    }

    TKF_IOA_METHOD_IMPL( LimitOrderResponse, ordersLimitOrder(const QString &figi, const OperationType &operation, qint32 nLots  , double price, QString broker_account_id = QString()) )
    {
        LimitOrderRequest request;
        request.setLots(nLots);
        request.setOperation(operation);
        request.setPrice(price);
        return ordersLimitOrder(figi, request, broker_account_id);
    }

    TKF_IOA_METHOD_IMPL( LimitOrderResponse, ordersLimitOrder(const QString &figi, const OperationType::eOperationType &operation, qint32 nLots, double price, QString broker_account_id = QString()) )
    {
        return ordersLimitOrder( figi, toOperationType(operation), nLots, price, broker_account_id);
    }

    TKF_IOA_METHOD_IMPL( LimitOrderResponse, ordersLimitOrder(const QString &figi, const QString &operation      , qint32 nLots  , double price, QString broker_account_id = QString()) )
    {
        return ordersLimitOrder( figi, toOperationType(operation), nLots, price, broker_account_id);
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( MarketOrderResponse, ordersMarketOrder(const QString &figi, const MarketOrderRequest &market_order_request, QString broker_account_id = QString()) )
    {
        checkBrokerAccountIdParam(broker_account_id);

        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( MarketOrderResponse, response );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( response.get(), m_pOrdersApi.get(), ordersMarketOrder, Post );
        m_pOrdersApi->ordersMarketOrderPost(figi, market_order_request, broker_account_id);
        return response;
    }

    TKF_IOA_METHOD_IMPL( MarketOrderResponse, ordersMarketOrder(const QString &figi, const OperationType &operation, qint32 nLots  , QString broker_account_id = QString()) )
    {
        MarketOrderRequest request;
        request.setLots(nLots);
        request.setOperation(operation);
        return ordersMarketOrder(figi, request, broker_account_id);
    }

    TKF_IOA_METHOD_IMPL( MarketOrderResponse, ordersMarketOrder(const QString &figi, const OperationType::eOperationType &operation, qint32 nLots, QString broker_account_id = QString()) )
    {
        return ordersMarketOrder( figi, toOperationType(operation), nLots, broker_account_id);
    }

    TKF_IOA_METHOD_IMPL( MarketOrderResponse, ordersMarketOrder(const QString &figi, const QString &operation      , qint32 nLots  , QString broker_account_id = QString()) )
    {
        return ordersMarketOrder( figi, toOperationType(operation), nLots, broker_account_id);
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( Empty, ordersCancel(const QString &order_id, QString broker_account_id = QString() ))
    {
        checkBrokerAccountIdParam(broker_account_id);

        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( Empty, response );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( response.get(), m_pOrdersApi.get(), ordersCancel, Post );
        m_pOrdersApi->ordersCancelPost(order_id,broker_account_id);
        return response;
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( OrdersResponse, orders(QString broker_account_id = QString() ))
    {
        checkBrokerAccountIdParam(broker_account_id);

        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( OrdersResponse, response );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( response.get(), m_pOrdersApi.get(), orders, Get );
        m_pOrdersApi->ordersGet(broker_account_id);
        return response;
    }

    //------------------------------


    //------------------------------
    // OperationsApi
    //------------------------------


    //------------------------------
    TKF_IOA_METHOD_IMPL( OperationsResponse , operations ( const QDateTime &from, const QDateTime &to, const QString &figi, QString broker_account_id = QString()) )
    {
        checkBrokerAccountIdParam(broker_account_id);

        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( OperationsResponse, response );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( response.get(), m_pOperationsApi.get(), operations, Get );
        m_pOperationsApi->operationsGet( from, to, figi, broker_account_id);
        return response;
    }

    bool findInstrumentListingStartDate( const QString &figi, const QDate &dateStartLookupFrom, QDate &foundDate ) override
    {
        bool bFound = false;
        QDateTime foundDateTime;

        QDate startDate = dateStartLookupFrom;
        QDate currentDate = QDate::currentDate();

        while(startDate < currentDate)
        {
            QDate nextDate = qt_helpers::addYearsNotGreaterThanDate( startDate, 10, currentDate );

            QDateTime requestBeginTime; requestBeginTime.setDate( startDate );
            QDateTime requestEndTime  ; requestEndTime  .setDate( nextDate );

            auto // CandlesResponse
            candlesRes = marketCandles( figi, requestBeginTime, requestEndTime, "MONTH" );

            candlesRes->join();

            if (candlesRes->isCompletionError())
                return false;

            QList<Candle> candleList = candlesRes->value.getPayload().getCandles();

            for( const auto &candle : candleList)
            {
                if (!candle.isSet() || !candle.isValid())
                     continue;
           
                QDateTime candleDt = candle.getTime();
           
                // Не факт, что данные отсортированы по дате
                // Поэтому сравниваем и не брякаемся, а проходим по всем
           
                if (!bFound)
                {
                    bFound = true;
                    foundDateTime = candleDt;
                }
                else
                {
                    if (foundDateTime>candleDt)
                        foundDateTime = candleDt;
                }

            } // for

            if (bFound)
                break;
           
            startDate = nextDate;

        } // while

        if (!bFound)
            return false;


        bFound = false;
        startDate = foundDateTime.date();

        if ( !startDate.setDate( startDate.year(), startDate.month(), 1) )
            return false;

        QDate nextDate = startDate.addMonths(1);
       
        QDateTime requestBeginTime; requestBeginTime.setDate( startDate );
        QDateTime requestEndTime  ; requestEndTime  .setDate( nextDate );
       
       
        auto // CandlesResponse
        candlesRes = marketCandles( "BBG004731354", requestBeginTime, requestEndTime, "DAY" );

        candlesRes->join();

        if (candlesRes->isCompletionError())
            return false;

        QList<Candle> candleList = candlesRes->value.getPayload().getCandles();

        for( const auto &candle : candleList)
        {
            if (!candle.isSet() || !candle.isValid())
                 continue;
        
            QDateTime candleDt = candle.getTime();
       
            // Не факт, что данные отсортированы по дате
       
            if (!bFound)
            {
                bFound = true;
                foundDateTime = candleDt;
            }
            else
            {
                if (foundDateTime>candleDt)
                    foundDateTime = candleDt;
            }
        }

        if (!bFound)
            return false;

        foundDate = foundDateTime.date();

        return true;
    }



protected:

    void mergeResponse( MarketInstrumentListResponse &mergeTo, const MarketInstrumentListResponse &mergeFrom )
    {
        QList<MarketInstrument> listTo = mergeTo.getPayload().getInstruments();
        listTo.append(mergeFrom.getPayload().getInstruments());
        MarketInstrumentList tmp;
        tmp.setInstruments(listTo);
        tmp.setTotal(listTo.size());
        //tmp.m_total_isValid = true;   // use Public Morozoff in models.h
        mergeTo.setPayload(tmp);
    }

    //------------------------------
    void initApis( const OpenApiFactory &factory )
    {
        m_pOrdersApi     = factory.getApiImpl< OrdersApi     >();
        m_pPortfolioApi  = factory.getApiImpl< PortfolioApi  >();
        m_pMarketApi     = factory.getApiImpl< MarketApi     >();
        m_pOperationsApi = factory.getApiImpl< OperationsApi >();
        m_pUserApi       = factory.getApiImpl< UserApi       >();
    }


    //------------------------------
    ApiConfig       m_apiConfig;
    AuthConfig      m_authConfig;
    LoggingConfig   m_loggingConfig;

    QSharedPointer<OrdersApi    >  m_pOrdersApi    ;
    QSharedPointer<PortfolioApi >  m_pPortfolioApi ;
    QSharedPointer<MarketApi    >  m_pMarketApi    ;
    QSharedPointer<OperationsApi>  m_pOperationsApi;
    QSharedPointer<UserApi      >  m_pUserApi      ;

    QString                        m_brokerAccountId;


}; // class OpenApiImpl

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
class SanboxOpenApiImpl : public OpenApiImpl
                        , public ISanboxOpenApi
{

    friend class OpenApiFactory;


    //------------------------------

public:
    
    //------------------------------
    SanboxOpenApiImpl( const ApiConfig  &apiConfig
                     , const AuthConfig &authConfig
                     , const LoggingConfig &loggingConfig
                     )
    : OpenApiImpl( apiConfig, authConfig, loggingConfig )
    {
        OpenApiFactory factory = OpenApiFactory(m_apiConfig, m_authConfig);
        m_pSandboxApi = factory.getApiImpl< SandboxApi >();
    }

    //------------------------------
    virtual void    setBrokerAccountId( const QString &id ) override
    {
        OpenApiImpl::setBrokerAccountId(id);
    }

    //------------------------------
    virtual QString getBrokerAccountId()                    override
    {
        return OpenApiImpl::getBrokerAccountId();
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( Empty, sandboxClear(QString broker_account_id = QString() ) )
    {
        checkBrokerAccountIdParam(broker_account_id);

        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( Empty, response );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( response.get(), m_pSandboxApi.get(), sandboxClear, Post );
        m_pSandboxApi->sandboxClearPost(broker_account_id);

        return response;
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( SandboxRegisterResponse, sandboxRegister(BrokerAccountType v) )
    {
        SandboxRegisterRequest sandboxRegisterRequest;
        sandboxRegisterRequest.setBrokerAccountType(v);

        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( SandboxRegisterResponse, sandboxRegisterResponse );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( sandboxRegisterResponse.get(), m_pSandboxApi.get(), sandboxRegister, Post );
        m_pSandboxApi->sandboxRegisterPost(sandboxRegisterRequest);

        return sandboxRegisterResponse;
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( SandboxRegisterResponse, sandboxRegister(BrokerAccountType::eBrokerAccountType v) )
    {
        BrokerAccountType brokerAccountType;
        brokerAccountType.setValue( v );
        return sandboxRegister(brokerAccountType);
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( Empty, sandboxCurrenciesBalanceSet(const SandboxCurrency &currency, Decimal balance, QString broker_account_id = QString() ) )
    {
        checkBrokerAccountIdParam(broker_account_id);

        SandboxSetCurrencyBalanceRequest request;

        //SandboxCurrency sandboxCurrency;
        //sandboxCurrency.setValue(currency);

        request.setCurrency(currency);
        request.setBalance(balance);

        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( Empty, response );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( response.get(), m_pSandboxApi.get(), sandboxCurrenciesBalance, Post );
        m_pSandboxApi->sandboxCurrenciesBalancePost(request, broker_account_id);
        
        return response;
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( Empty, sandboxCurrenciesBalanceSet(const SandboxCurrency::eSandboxCurrency currency, Decimal balance, QString broker_account_id = QString() ) )
    {
        checkBrokerAccountIdParam(broker_account_id);

        SandboxSetCurrencyBalanceRequest request;

        SandboxCurrency sandboxCurrency;
        sandboxCurrency.setValue(currency);

        request.setCurrency(sandboxCurrency);
        request.setBalance(balance);

        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( Empty, response );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( response.get(), m_pSandboxApi.get(), sandboxCurrenciesBalance, Post );
        m_pSandboxApi->sandboxCurrenciesBalancePost(request, broker_account_id);
        
        return response;
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( Empty, sandboxCurrenciesBalanceSet(const QVector<CurrencyConfig> currencies, QString broker_account_id = QString() ) )
    {
        if (currencies.isEmpty())
        {
            throw std::runtime_error("sandboxCurrenciesBalanceSet: at least one currency must be taken");
        }

        QVector< QSharedPointer< OpenApiCompletableFuture< Empty > > > results;
        for( const auto &currency : currencies)
        {
            results.push_back( sandboxCurrenciesBalanceSet( currency.getType(), currency.getValue(), broker_account_id ) );
            pollMessageQueue();
        }

        joinOpenApiCompletableFutures( results );

        for( const auto &res : results)
        {
            if (res->isCompletionError()) // return first bad, if exist
                return res;
        }

        return results.front(); // return first
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( Empty, sandboxPositionsBalanceSet(const SandboxSetPositionBalanceRequest          &positionBalance , QString broker_account_id = QString() ) )
    {
        checkBrokerAccountIdParam(broker_account_id);

        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( Empty, response );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( response.get(), m_pSandboxApi.get(), sandboxPositionsBalance, Post );
        m_pSandboxApi->sandboxPositionsBalancePost(positionBalance, broker_account_id);

        return response;
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( Empty, sandboxPositionsBalanceSet(const QString &positionFigi, const marty::Decimal &positionBalance, QString broker_account_id = QString() ) )
    {
        SandboxSetPositionBalanceRequest request;
        request.setFigi(positionFigi);
        request.setBalance(positionBalance);
        return sandboxPositionsBalanceSet(request,broker_account_id);
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( Empty, sandboxPositionsBalanceSet(const QVector<SandboxSetPositionBalanceRequest> &positionBalances, QString broker_account_id = QString() ) )
    {
        if (positionBalances.isEmpty())
        {
            throw std::runtime_error("sandboxPositionsBalanceSet: at least one position must be taken");
        }

        QVector< QSharedPointer< OpenApiCompletableFuture< Empty > > > results;
        for( const auto &positionBalance : positionBalances)
        {
            results.push_back( sandboxPositionsBalanceSet( positionBalance, broker_account_id ) );
            pollMessageQueue();
        }

        joinOpenApiCompletableFutures( results );

        for( const auto &res : results)
        {
            if (res->isCompletionError()) // return first bad, if exist
                return res;
        }

        return results.front(); // return first
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( Empty, sandboxRemove(QString broker_account_id = QString()) )
    {
        checkBrokerAccountIdParam(broker_account_id);

        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( Empty, response );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( response.get(), m_pSandboxApi.get(), sandboxRemove, Post );
        m_pSandboxApi->sandboxRemovePost(broker_account_id);

        return response;
    
    }

    //------------------------------


protected:


    QSharedPointer<SandboxApi>  m_pSandboxApi;


}; // class SanboxOpenApiImpl

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
inline
QSharedPointer<IOpenApi>
createOpenApi( const ApiConfig  &apiConfig
             , const AuthConfig &authConfig
             , const LoggingConfig &loggingConfig
             )
{
    OpenApiFactory faq = OpenApiFactory(apiConfig, authConfig);

    IOpenApi *pApi = authConfig.sandboxMode
                   ? static_cast<IOpenApi*>(static_cast<OpenApiImpl*>(new SanboxOpenApiImpl(apiConfig, authConfig, loggingConfig)))
                   : static_cast<IOpenApi*>(new OpenApiImpl(apiConfig, authConfig, loggingConfig))
                   ;

    return QSharedPointer<IOpenApi>( pApi );
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
inline
ISanboxOpenApi* getSandboxApi( IOpenApi* pApi )
{
    return dynamic_cast<ISanboxOpenApi*>(pApi);
}

//------------------------------
inline
ISanboxOpenApi* getSandboxApi( QSharedPointer<IOpenApi> pApi )
{
    return getSandboxApi(pApi.get());
}

//----------------------------------------------------------------------------


} // namespace invest_openapi

