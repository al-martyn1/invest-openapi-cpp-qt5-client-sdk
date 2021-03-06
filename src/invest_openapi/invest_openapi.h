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


using marty::Decimal         ;


enum class GenericError
{
    ok,
    internalError,
    networkError,
    noData
};



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

    virtual ~IOpenApi() {};

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
    TKF_IOA_ABSTRACT_METHOD( LimitOrderResponse , ordersLimitOrder (const QString &figi, const OperationType &operation, qint32 nLots  , marty::Decimal price, QString broker_account_id = QString()) );
    TKF_IOA_ABSTRACT_METHOD( LimitOrderResponse , ordersLimitOrder (const QString &figi, const OperationType::eOperationType &operation, qint32 nLots, marty::Decimal price, QString broker_account_id = QString()) );
    TKF_IOA_ABSTRACT_METHOD( LimitOrderResponse , ordersLimitOrder (const QString &figi, const QString &operation      , qint32 nLots  , marty::Decimal price, QString broker_account_id = QString()) );

    TKF_IOA_ABSTRACT_METHOD( MarketOrderResponse, ordersMarketOrder(const QString &figi, const MarketOrderRequest &market_order_request, QString broker_account_id = QString()) );
    TKF_IOA_ABSTRACT_METHOD( MarketOrderResponse, ordersMarketOrder(const QString &figi, const OperationType &operation, qint32 nLots  , QString broker_account_id = QString()) );
    TKF_IOA_ABSTRACT_METHOD( MarketOrderResponse, ordersMarketOrder(const QString &figi, const OperationType::eOperationType &operation, qint32 nLots, QString broker_account_id = QString()) );
    TKF_IOA_ABSTRACT_METHOD( MarketOrderResponse, ordersMarketOrder(const QString &figi, const QString &operation      , qint32 nLots  , QString broker_account_id = QString()) );

    TKF_IOA_ABSTRACT_METHOD( Empty, ordersCancel(const QString &order_id, QString broker_account_id = QString() ));
    TKF_IOA_ABSTRACT_METHOD( OrdersResponse, orders(QString broker_account_id = QString() ));

    //------------------------------
    // OperationsApi

    TKF_IOA_ABSTRACT_METHOD( OperationsResponse , operations ( const QDateTime &from, const QDateTime &to, const QString &figi, QString broker_account_id = QString()) );
    TKF_IOA_ABSTRACT_METHOD( OperationsResponse , operations ( QString intervalStr /* eg "10YEAR" */, const QDateTime &to, const QString &figi, QString broker_account_id = QString()) );
    TKF_IOA_ABSTRACT_METHOD( OperationsResponse , operations ( QString intervalStr /* eg "10YEAR" */ /* , const QDateTime &to = now */ , const QString &figi, QString broker_account_id = QString()) );

/*
    void operationsGet(const QDateTime &from, const QDateTime &to, const QString &figi, const QString &broker_account_id);
    void operationsGetSignal(OperationsResponse summary);

*/

    //------------------------------
    // StreamingApi

    virtual QNetworkRequest getStreamingApiNetworkRequest() = 0;

    // https://tinkoffcreditsystems.github.io/invest-openapi/marketdata/

    /*
      orderbook:subscribe

          {
              "event": "orderbook:subscribe",
              "figi": "{{FIGI}}",
              "depth": {{DEPTH}}
          }
       

      orderbook:unsubscribe

          {
              "event": "orderbook:unsubscribe",
              "figi": "{{FIGI}}",
              "depth": "{{DEPTH}}"
          }

      Глубина стакана макс 20
          
    */

    virtual QString getStreamingApiOrderbookSubscribeJson  ( const QString &figi, qint32 depth = 20 ) = 0;
    virtual QString getStreamingApiOrderbookUnsubscribeJson( const QString &figi, qint32 depth = 20 ) = 0; // Вот тут точно нужна глубина корзины? Ага, нужна, бля


    virtual QString getStreamingApiInstrumentInfoSubscribeJson  ( const QString &figi ) = 0;
    virtual QString getStreamingApiInstrumentInfoUnsubscribeJson( const QString &figi ) = 0;


    virtual QString getStreamingApiCandleSubscribeJson  ( const QString &figi, const CandleResolution &interval             ) = 0;
    virtual QString getStreamingApiCandleSubscribeJson  ( const QString &figi, CandleResolution::eCandleResolution interval ) = 0;
    virtual QString getStreamingApiCandleSubscribeJson  ( const QString &figi, const QString &interval                      ) = 0;

    virtual QString getStreamingApiCandleUnsubscribeJson( const QString &figi, const CandleResolution &interval             ) = 0;
    virtual QString getStreamingApiCandleUnsubscribeJson( const QString &figi, CandleResolution::eCandleResolution interval ) = 0;
    virtual QString getStreamingApiCandleUnsubscribeJson( const QString &figi, const QString &interval                      ) = 0;



    //------------------------------
    // Helpers
    virtual GenericError findInstrumentListingStartDate( const QString &figi, const QDate &dateStartLookupFrom, QDate &foundDate ) = 0;


}; // struct IOpenApi

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
struct ISanboxOpenApi // : public IOpenApi
{
    virtual ~ISanboxOpenApi() {};

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
    // StreamingApi
    //------------------------------


    //------------------------------

protected:

    static QByteArray makeByteArray( const std::string &s )
    {
        return QByteArray( s.data(), s.size() );
    }

    static QByteArray makeByteArray( const QString &s )
    {
        return makeByteArray( s.toStdString() );
    }

    static void setNetworkRequestRawHeader( QNetworkRequest &req, const QString &headerName, const QString &headerValue )
    {
        req.setRawHeader( makeByteArray(headerName), makeByteArray(headerValue) );
    }


public:

    virtual QNetworkRequest getStreamingApiNetworkRequest() override
    {
        QNetworkRequest request;

        request.setUrl( QUrl(m_apiConfig.urlStreaming) );

        setNetworkRequestRawHeader( request, "User-Agent"     , "OpenAPI-Generator/1.0.0/cpp-qt5" );
        setNetworkRequestRawHeader( request, "Authorization"  , QString("Bearer ") + m_authConfig.getToken() );
        setNetworkRequestRawHeader( request, "Accept"         , "application/json" );

        // "Content-Type" : "application/json"  /  "application/x-www-form-urlencoded"  /  "multipart/form-data; boundary=" + boundary

        bool isRequestCompressionEnabled  = false;
        bool isResponseCompressionEnabled = false;

        if (isRequestCompressionEnabled)
        {
            setNetworkRequestRawHeader( request, "Content-Encoding", "gzip" );
        }
        else
        {
        }

        if (isResponseCompressionEnabled)
        {
            setNetworkRequestRawHeader( request, "Accept-Encoding", "gzip" );
        }
        else
        {
            setNetworkRequestRawHeader( request, "Accept-Encoding", "identity" );
        }

        
        // setNetworkRequestRawHeader( request, "", "" );

        return request;
    }

protected:

    static QString makeJsonValue( const QString &s )
    {
        return QString("\"") + s + QString("\"");
    }

    static QString makeJsonValue( qint32 qi32 )
    {
        return QString::number(qi32);
    }

    static QString getStreamingApiOrderbookJsonSentenceHelper( const QString &sentence, const QString &figi, qint32 depth )
    {
        if (depth>20)
            depth = 20;

        QString eventStr = makeJsonValue( QStringLiteral("event" ) ) + QStringLiteral(": ") + makeJsonValue( sentence );
        QString figiStr  = makeJsonValue( QStringLiteral("figi"  ) ) + QStringLiteral(": ") + makeJsonValue( figi  );

        if (depth<0)
        {
            return QStringLiteral("{\r\n") + eventStr + QStringLiteral(",\r\n") + figiStr + QStringLiteral("\r\n}");
        }
        else
        {
            return QStringLiteral("{\r\n") + eventStr + QStringLiteral(",\r\n") + figiStr + QStringLiteral(",\r\n")
                 + makeJsonValue( QStringLiteral("depth" ) ) + QStringLiteral(": ") + makeJsonValue( depth ) + QStringLiteral("\r\n}");
        }
    }

    static QString getStreamingApiInstrumentInfoJsonSentenceHelper( const QString &sentence, const QString &figi )
    {
        QString eventStr = makeJsonValue( QStringLiteral("event" ) ) + QStringLiteral(": ") + makeJsonValue( sentence );
        QString figiStr  = makeJsonValue( QStringLiteral("figi"  ) ) + QStringLiteral(": ") + makeJsonValue( figi  );

        return QStringLiteral("{\r\n") + eventStr + QStringLiteral(",\r\n") + figiStr + QStringLiteral("\r\n}");
    }

    static QString getStreamingApiCandleJsonSentenceHelper( const QString &sentence, const QString &figi, const QString &candleInterval )
    {
        QString eventStr    = makeJsonValue( QStringLiteral("event" ) )     + QStringLiteral(": ") + makeJsonValue( sentence );
        QString figiStr     = makeJsonValue( QStringLiteral("figi"  ) )     + QStringLiteral(": ") + makeJsonValue( figi  );
        QString intervalStr = makeJsonValue( QStringLiteral("interval"  ) ) + QStringLiteral(": ") + makeJsonValue( candleInterval );

        return QStringLiteral("{\r\n") + eventStr + QStringLiteral(",\r\n") + figiStr + QStringLiteral(",\r\n") + intervalStr + QStringLiteral("\r\n}");
    }

public:

    //! Формирует JSON-посылку для подписки на стакан по streaming API. Глубина стакана макс 20
    virtual QString getStreamingApiOrderbookSubscribeJson  ( const QString &figi, qint32 depth = 20    ) override
    {
        return getStreamingApiOrderbookJsonSentenceHelper( QStringLiteral("orderbook:subscribe"), figi, depth );
    }

    //! Формирует JSON-посылку для подписки на стакан по streaming API
    /*! Какого-то хуя при отписке нужно указывать глубину стакана.
        Лень проверять, но вероятно на серваке это выглядит, как абсолютно разные подписки, 
        и сервак будет слать две посылки с разной глубиной стакана.
        Как-то тупо, но хрен с ними, шо маемо, то маемо.

        Если depth при отписке задать в -1, то глубина стакана вообще не появится в запросе на отписку (на отписку, Карл),
        и серваке шлёт ошибку (просто хотел проверить).
     */
    virtual QString getStreamingApiOrderbookUnsubscribeJson( const QString &figi, qint32 depth = 20 ) override
    {
        return getStreamingApiOrderbookJsonSentenceHelper( QStringLiteral("orderbook:unsubscribe"), figi, depth );
    }


    virtual QString getStreamingApiInstrumentInfoSubscribeJson  ( const QString &figi ) override
    {
        return getStreamingApiInstrumentInfoJsonSentenceHelper( "instrument_info:subscribe", figi );
    }

    virtual QString getStreamingApiInstrumentInfoUnsubscribeJson( const QString &figi ) override
    {
        return getStreamingApiInstrumentInfoJsonSentenceHelper( "instrument_info:unsubscribe", figi );
    }


    virtual QString getStreamingApiCandleSubscribeJson  ( const QString &figi, const CandleResolution &interval             ) override
    {
        return getStreamingApiCandleJsonSentenceHelper( "candle:subscribe", figi, interval.asJson() );
    }

    virtual QString getStreamingApiCandleSubscribeJson  ( const QString &figi, CandleResolution::eCandleResolution interval ) override
    {
        return getStreamingApiCandleSubscribeJson( figi, toCandleResolution(interval) );
    }

    virtual QString getStreamingApiCandleSubscribeJson  ( const QString &figi, const QString &interval                      ) override
    {
        return getStreamingApiCandleSubscribeJson( figi, toCandleResolution(interval) );
    }


    virtual QString getStreamingApiCandleUnsubscribeJson( const QString &figi, const CandleResolution &interval             ) override
    {
        return getStreamingApiCandleJsonSentenceHelper( "candle:unsubscribe", figi, interval.asJson() );
    }

    virtual QString getStreamingApiCandleUnsubscribeJson( const QString &figi, CandleResolution::eCandleResolution interval ) override
    {
        return getStreamingApiCandleUnsubscribeJson( figi, toCandleResolution(interval) );
    }

    virtual QString getStreamingApiCandleUnsubscribeJson( const QString &figi, const QString &interval                      ) override
    {
        return getStreamingApiCandleUnsubscribeJson( figi, toCandleResolution(interval) );
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
        response->setWorker(m_pMarketApi->marketBondsGet());

        return response;
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( MarketInstrumentListResponse, marketCurrencies() )
    {
        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( MarketInstrumentListResponse, response );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( response.get(), m_pMarketApi.get(), marketCurrencies, Get );
        response->setWorker(m_pMarketApi->marketCurrenciesGet());

        return response;
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( MarketInstrumentListResponse, marketEtfs() )
    {
        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( MarketInstrumentListResponse, response );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( response.get(), m_pMarketApi.get(), marketEtfs, Get );
        response->setWorker(m_pMarketApi->marketEtfsGet());

        return response;
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( MarketInstrumentListResponse, marketStocks() )
    {
        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( MarketInstrumentListResponse, response );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( response.get(), m_pMarketApi.get(), marketStocks, Get );
        response->setWorker(m_pMarketApi->marketStocksGet());

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
        response->setWorker(m_pMarketApi->marketCandlesGet(figi, from, to, interval));
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
        response->setWorker(m_pMarketApi->marketOrderbookGet(figi, depth));
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
        response->setWorker(m_pUserApi->userAccountsGet());
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
        response->setWorker(m_pPortfolioApi->portfolioCurrenciesGet(broker_account_id));
        return response;
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( PortfolioResponse, portfolio(QString broker_account_id = QString()) )
    {
        checkBrokerAccountIdParam(broker_account_id);

        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( PortfolioResponse, response );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( response.get(), m_pPortfolioApi.get(), portfolio, Get );
        response->setWorker(m_pPortfolioApi->portfolioGet(broker_account_id));
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
        response->setWorker(m_pOrdersApi->ordersLimitOrderPost(figi, limit_order_request, broker_account_id));
        return response;
    }

    TKF_IOA_METHOD_IMPL( LimitOrderResponse, ordersLimitOrder(const QString &figi, const OperationType &operation, qint32 nLots  , marty::Decimal price, QString broker_account_id = QString()) )
    {
        LimitOrderRequest request;
        request.setLots(nLots);
        request.setOperation(operation);
        request.setPrice(price);
        return ordersLimitOrder(figi, request, broker_account_id);
    }

    TKF_IOA_METHOD_IMPL( LimitOrderResponse, ordersLimitOrder(const QString &figi, const OperationType::eOperationType &operation, qint32 nLots, marty::Decimal price, QString broker_account_id = QString()) )
    {
        return ordersLimitOrder( figi, toOperationType(operation), nLots, price, broker_account_id);
    }

    TKF_IOA_METHOD_IMPL( LimitOrderResponse, ordersLimitOrder(const QString &figi, const QString &operation      , qint32 nLots  , marty::Decimal price, QString broker_account_id = QString()) )
    {
        return ordersLimitOrder( figi, toOperationType(operation), nLots, price, broker_account_id);
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( MarketOrderResponse, ordersMarketOrder(const QString &figi, const MarketOrderRequest &market_order_request, QString broker_account_id = QString()) )
    {
        checkBrokerAccountIdParam(broker_account_id);

        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( MarketOrderResponse, response );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( response.get(), m_pOrdersApi.get(), ordersMarketOrder, Post );
        response->setWorker(m_pOrdersApi->ordersMarketOrderPost(figi, market_order_request, broker_account_id));
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
        response->setWorker(m_pOrdersApi->ordersCancelPost(order_id,broker_account_id));
        return response;
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( OrdersResponse, orders(QString broker_account_id = QString() ))
    {
        checkBrokerAccountIdParam(broker_account_id);

        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( OrdersResponse, response );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( response.get(), m_pOrdersApi.get(), orders, Get );
        response->setWorker(m_pOrdersApi->ordersGet(broker_account_id));
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
        response->setWorker(m_pOperationsApi->operationsGet( from, to, figi, broker_account_id));
        return response;
    }


    TKF_IOA_METHOD_IMPL( OperationsResponse , operations ( QString intervalStr /* eg "10YEAR" */, const QDateTime &to, const QString &figi, QString broker_account_id = QString()) )
    {
        checkBrokerAccountIdParam(broker_account_id);

        if (intervalStr.isEmpty())
            intervalStr = "1YEAR";

        QDateTime dateTimeFrom = qt_helpers::dtAddTimeInterval( to, intervalStr, -1 /* forceSign */ );

        return operations( dateTimeFrom, to, figi, broker_account_id );
    }

    TKF_IOA_METHOD_IMPL( OperationsResponse , operations ( QString intervalStr /* eg "10YEAR" */ /* , const QDateTime &to = now */ , const QString &figi, QString broker_account_id = QString()) )
    {
        checkBrokerAccountIdParam(broker_account_id);

        if (intervalStr.isEmpty())
            intervalStr = "1YEAR";

        QDateTime dateTimeNow     = QDateTime::currentDateTime();

        return operations( intervalStr, dateTimeNow, figi, broker_account_id );
    }


    GenericError findInstrumentListingStartDate( const QString &figi, const QDate &dateStartLookupFrom, QDate &foundDate ) override
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
                return GenericError::networkError;

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
            return GenericError::noData;


        bFound = false;
        startDate = foundDateTime.date();

        if ( !startDate.setDate( startDate.year(), startDate.month(), 1) )
            return GenericError::internalError;

        QDate nextDate = startDate.addMonths(1);
       
        QDateTime requestBeginTime; requestBeginTime.setDate( startDate );
        QDateTime requestEndTime  ; requestEndTime  .setDate( nextDate );
       
       
        auto // CandlesResponse
        candlesRes = marketCandles( "BBG004731354", requestBeginTime, requestEndTime, "DAY" );

        candlesRes->join();

        if (candlesRes->isCompletionError())
            return GenericError::networkError;

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
            return GenericError::noData;

        foundDate = foundDateTime.date();

        return GenericError::ok;
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
        response->setWorker(m_pSandboxApi->sandboxClearPost(broker_account_id));

        return response;
    }

    //------------------------------
    TKF_IOA_METHOD_IMPL( SandboxRegisterResponse, sandboxRegister(BrokerAccountType v) )
    {
        SandboxRegisterRequest sandboxRegisterRequest;
        sandboxRegisterRequest.setBrokerAccountType(v);

        TKF_IOA_NEW_SHARED_COMPLETABLE_FUTURE( SandboxRegisterResponse, response );
        INVEST_OPENAPI_COMPLETABLE_FUTURE_CONNECT_TO_API( response.get(), m_pSandboxApi.get(), sandboxRegister, Post );
        response->setWorker(m_pSandboxApi->sandboxRegisterPost(sandboxRegisterRequest));

        return response;
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
        response->setWorker(m_pSandboxApi->sandboxCurrenciesBalancePost(request, broker_account_id));
        
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
        response->setWorker(m_pSandboxApi->sandboxCurrenciesBalancePost(request, broker_account_id));
        
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
        response->setWorker(m_pSandboxApi->sandboxPositionsBalancePost(positionBalance, broker_account_id));

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
        response->setWorker(m_pSandboxApi->sandboxRemovePost(broker_account_id));

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






//----------------------------------------------------------------------------
inline
void AuthConfig::setDefaultBrokerAccountForOpenApi( QSharedPointer<IOpenApi> pOpenApi ) const
{
    ISanboxOpenApi* pSandboxOpenApi = getSandboxApi(pOpenApi);
   
    if (pSandboxOpenApi)
    {
        pSandboxOpenApi->setBrokerAccountId( getBrokerAccountId() );
    }
    else
    {
        pOpenApi->setBrokerAccountId( getBrokerAccountId() );
    }

}








} // namespace invest_openapi

