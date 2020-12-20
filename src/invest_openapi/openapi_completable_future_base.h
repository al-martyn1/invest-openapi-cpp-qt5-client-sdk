#pragma once

#include <QObject>
#include <QNetworkReply>
#include <QTest>

#include <atomic>

#include "models.h"
#include "invest_openapi.h"


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


protected:

    void errorComplete( QNetworkReply::NetworkError errorType    = QNetworkReply::NoError
                 , QString                     errorMessage = QString()
                 )
    {
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

    virtual void onComplete(Empty v)                   {}
    virtual void onComplete(SandboxRegisterResponse v) {}

    virtual void onError(Empty                   v, QNetworkReply::NetworkError et, QString es) { errorComplete( et, es); }
    virtual void onError(SandboxRegisterResponse v, QNetworkReply::NetworkError et, QString es) { errorComplete( et, es); }
    


}; // class OpenApiCompletableFuture


} // namespace invest_openapi


