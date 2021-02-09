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

        qDebug().nospace().noquote() << "!!! (+) " << __FUNCTION_NAME__ ;

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

    virtual void onComplete( Empty                          v ) {}

    virtual void onComplete( SandboxRegisterResponse        v ) {}

    virtual void onComplete( MarketInstrumentListResponse   v ) {}

    virtual void onComplete( CandlesResponse                v ) {}
    virtual void onComplete( OrderbookResponse              v ) {}
    virtual void onComplete( SearchMarketInstrumentResponse v ) {}

    virtual void onComplete( UserAccountsResponse           v ) {}

    virtual void onComplete( OperationsResponse             v ) {}

    virtual void onComplete( OrdersResponse                 v ) {}
    virtual void onComplete( LimitOrderResponse             v ) {}
    virtual void onComplete( MarketOrderResponse            v ) {}

    virtual void onComplete( PortfolioCurrenciesResponse    v ) {}
    virtual void onComplete( PortfolioResponse              v ) {}

    

    virtual void onError( Empty                          v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }

    virtual void onError( SandboxRegisterResponse        v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }

    virtual void onError( MarketInstrumentListResponse   v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }

    virtual void onError( CandlesResponse                v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }
    virtual void onError( OrderbookResponse              v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }
    virtual void onError( SearchMarketInstrumentResponse v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }

    virtual void onError( UserAccountsResponse           v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }

    virtual void onError( OperationsResponse             v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }

    virtual void onError( OrdersResponse                 v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }
    virtual void onError( LimitOrderResponse             v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }
    virtual void onError( MarketOrderResponse            v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }

    virtual void onError( PortfolioCurrenciesResponse    v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }
    virtual void onError( PortfolioResponse              v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }
    


}; // class OpenApiCompletableFuture


} // namespace invest_openapi


