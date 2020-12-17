#pragma once

#include <QObject>
#include <QNetworkReply>
#include <QTest>

#include <atomic>

#include "models.h"

namespace invest_openapi
{

class NetworkCompletableFutureBase : public QObject
{

    Q_OBJECT

public:

    bool isCompleted() const
    {
        // https://habr.com/ru/post/517918/
        return m_complete.load(std::memory_order_relaxed);
    }

    bool isCompletionError() const
    {
        return m_errorType != QNetworkReply::NoError;
    }

    

protected slots:

    virtual void onComplete(Empty v) {}
    virtual void onComplete(SandboxRegisterResponse v) {}

    virtual void onError(Empty                   v, QNetworkReply::NetworkError et, QString es) { errorComplete( v, et, es); }
    virtual void onError(SandboxRegisterResponse v, QNetworkReply::NetworkError et, QString es) { errorComplete( v, et, es); }
    


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
            QTest::qWait(0);
        }
    }


protected:

    std::atomic<bool>           m_complete     = false;
    QNetworkReply::NetworkError m_errorType    = QNetworkReply::NoError;
    QString                     m_errorMessage ;

    

}; // class NetworkCompletableFuture


} // namespace invest_openapi


