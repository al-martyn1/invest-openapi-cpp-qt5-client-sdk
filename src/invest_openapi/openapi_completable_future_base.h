/*! \file
    \brief 
 */

#pragma once

#include <QObject>
#include <QNetworkReply>
#include <QTest>
#include <QDebug>

#include <atomic>

#include "models.h"
#include "func.h"
#include "lib_config.h"



namespace invest_openapi
{


void pollMessageQueue();

template<typename ValueType> class OpenApiCompletableFuture;


class OpenApiCompletableFutureBase : public QObject
{

    Q_OBJECT

public:

    template<typename ValueType> friend class OpenApiCompletableFuture;

    bool isCompleted() const
    {
        // https://habr.com/ru/post/517918/
        return m_complete.load(std::memory_order_relaxed);
    }

    bool isCompletionError() const
    {
        return m_errorType != QNetworkReply::NoError;
    }

    QNetworkReply::NetworkError getError() const
    {
        return m_errorType;
    }

    QString getErrorMessage() const
    {
        return m_errorMessage;
    }



protected:

    void errorComplete( QNetworkReply::NetworkError errorType    = QNetworkReply::NoError
                 , QString                     errorMessage = QString()
                 )
    {

        #if defined(INVEST_OPENAPI_DEBUG_DUMP_COMPLETABLE_FUTURE_HANDLER_VALUES)
            qDebug().nospace().noquote() << "!!! (+) " << __FUNCTION_NAME__ ;
        #endif

        m_errorType     = errorType   ;
        m_errorMessage  = errorMessage;
        m_complete.store(true, std::memory_order_relaxed);
    }

    void joinImpl() const
    {
        while(!isCompleted())
        {
            pollMessageQueue();
        }
    }


protected:

    std::atomic<bool>           m_complete     = false;
    QNetworkReply::NetworkError m_errorType    = QNetworkReply::NoError;
    QString                     m_errorMessage ;

    
protected slots:

    virtual void onComplete( const Empty                          &v ) {}

    virtual void onComplete( const SandboxRegisterResponse        &v ) {}

    virtual void onComplete( const MarketInstrumentListResponse   &v ) {}

    virtual void onComplete( const CandlesResponse                &v ) {}
    virtual void onComplete( const OrderbookResponse              &v ) {}
    virtual void onComplete( const SearchMarketInstrumentResponse &v ) {}

    virtual void onComplete( const UserAccountsResponse           &v ) {}

    virtual void onComplete( const OperationsResponse             &v ) {}

    virtual void onComplete( const OrdersResponse                 &v ) {}
    virtual void onComplete( const LimitOrderResponse             &v ) {}
    virtual void onComplete( const MarketOrderResponse            &v ) {}

    virtual void onComplete( const PortfolioCurrenciesResponse    &v ) {}
    virtual void onComplete( const PortfolioResponse              &v ) {}

    

    virtual void onError   ( const Empty                          &v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }

    virtual void onError   ( const SandboxRegisterResponse        &v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }

    virtual void onError   ( const MarketInstrumentListResponse   &v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }

    virtual void onError   ( const CandlesResponse                &v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }
    virtual void onError   ( const OrderbookResponse              &v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }
    virtual void onError   ( const SearchMarketInstrumentResponse &v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }

    virtual void onError   ( const UserAccountsResponse           &v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }

    virtual void onError   ( const OperationsResponse             &v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }

    virtual void onError   ( const OrdersResponse                 &v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }
    virtual void onError   ( const LimitOrderResponse             &v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }
    virtual void onError   ( const MarketOrderResponse            &v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }

    virtual void onError   ( const PortfolioCurrenciesResponse    &v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }
    virtual void onError   ( const PortfolioResponse              &v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }
    


}; // class OpenApiCompletableFuture


} // namespace invest_openapi


