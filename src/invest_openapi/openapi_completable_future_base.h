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

//#include "utility.h"



namespace invest_openapi
{


void pollMessageQueue();
QString mergeString(const QVector<QString> &v, const QString &sep );

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

    bool isFinished() const // QFuture compatible
    {
        return isCompleted();
    }

    bool isCompletionError() const
    {
        return m_errorType != QNetworkReply::NoError;
    }

    QNetworkReply::NetworkError getError() const
    {
        return m_errorType;
    }

    //OpenAPI::Empty getErrorAsEmpty() const
    OpenAPI::Error getErrorAsObject() const
    {
        QString fullMsg = getErrorMessageInternal();

        int jsonStartPos = fullMsg.indexOf('{');

        if (jsonStartPos<0)
            return OpenAPI::Error();
            //return fullMsg;

        //QString getStatus() const;
        //Error_payload getPayload() const;
        //Error_payload getPayload() const;
        // Error_payload getMessage()
        //               getCode()

        fullMsg.remove(0, jsonStartPos);

        // Error
        //OpenAPI::Empty e = OpenAPI::Empty( fullMsg );
        OpenAPI::Error e = OpenAPI::Error( fullMsg ); // from JSON

        return e;

        // Error
        //   QString getTrackingId()
        //   QString getStatus()
        //   Error_payload getPayload()

        // Error_payload
        //   QString getMessage()
        //   QString getCode()
    }

protected:

    QString composeMsgStr( const QString &msg, const QString &codeStr = QString(), const QString &statusStr = QString() ) const
    {
        QVector<QString> resVec;

        if (!statusStr.isEmpty())
            resVec.push_back("Status: " + statusStr );

        if (!codeStr.isEmpty())
            resVec.push_back("Code: " + codeStr );

        resVec.push_back("Message: " + msg );

        return mergeString(resVec, ", ");

    }

public:

    QString getErrorMessage() const
    {
        QString fullMsg = getErrorMessageInternal();

        static const QString errStart  = ",\"payload\":{\"message\":";
        static const QString codeStart = "\",\"code\":\"";
                                      // ","code":"
        //static const QString codeEnd   = "\"},\"status\"";
        static const QString codeEnd   = "\"},\"status\":\"";

        static const QString statusEnd = "\"}";

        QString codeStr, statusStr, msg = fullMsg;

        QStringList l1 = fullMsg.split(errStart);
        if (l1.size()>1)
        {
            msg = l1[1];
            QStringList l2 = msg.split(codeStart);
            if (l2.size()>1)
            {
                msg = l2[0];
                codeStr = l2[1];
                QStringList l3 = codeStr.split(codeEnd);
                if (l3.size()>1)
                {
                    codeStr = l3[0];
                    statusStr = l3[1];
                    QStringList l4 = statusStr.split(statusEnd);
                    if (l4.size()>=1)
                    {
                        statusStr = l4[0];
                    }
                }
            }
        }

        return composeMsgStr( msg, codeStr, statusStr );

        /*
        int msgStartIdx = fullMsg.indexOf(errStart);

        if (msgStartIdx<0)
            return fullMsg;

        fullMsg.remove(0, msgStartIdx+errStart.size());

        int codeStartIdx = fullMsg.indexOf(codeStart);
        if (codeStartIdx<0)
            return fullMsg;

        //QString msg       = fullMsg;
        QString msgFinale = fullMsg; // msg;
        msgFinale.remove(codeStartIdx, -1); // удалили всё, что было после кода ошибки

        fullMsg.remove(codeStartIdx, -1); // удалили всё, что было до кода ошибки

        int codeEndIdx = fullMsg.indexOf(codeEnd);
        if (codeEndIdx<0)
        {
            return msgFinale;
        }

        fullMsg.remove(codeEndIdx, -1);

        return fullMsg + " - " + msgFinale;
        */

        /*
        int jsonStartPos = fullMsg.indexOf('{');

        if (jsonStartPos<0)
            return fullMsg;

        fullMsg.remove(0, jsonStartPos);

        return fullMsg;
        */

        // Something goes wrong with json
        /*
        
        OpenAPI::Error e = getErrorAsObject();

        auto pl = e.getPayload();

        QString codeStr = pl.getCode();
        QString msgStr  = pl.getMessage();

        return QString("(") + codeStr + QString(") - ") + msgStr;
        */
    }

    // crutch/crotch
    bool checkErrorMessageFor( Qt::CaseSensitivity cs, const std::vector<QString> &stringsToCheck ) const
    {
        QString errMsg = getErrorMessage();

        for( const auto s : stringsToCheck )
        {
            //indexOf(const QString &str, int from = 0, Qt::CaseSensitivity cs = Qt::CaseSensitive) const
            //if ( errMsg.compare( s, cs) != 0 )
            if (errMsg.indexOf( s, 0, cs ) < 0 )
                return false;
        }

        return true;
    }

    bool checkErrorMessageFor( Qt::CaseSensitivity cs, const QString &s1 ) const { std::vector<QString>res; res.push_back(s1); return checkErrorMessageFor(cs,res); }
    bool checkErrorMessageFor( Qt::CaseSensitivity cs, const QString &s1, const QString &s2 ) const { std::vector<QString>res; res.push_back(s1); res.push_back(s2); return checkErrorMessageFor(cs,res); }
    bool checkErrorMessageFor( Qt::CaseSensitivity cs, const QString &s1, const QString &s2, const QString &s3 ) const { std::vector<QString>res; res.push_back(s1); res.push_back(s2); res.push_back(s3); return checkErrorMessageFor(cs,res); }
    bool checkErrorMessageFor( Qt::CaseSensitivity cs, const QString &s1, const QString &s2, const QString &s3, const QString &s4 ) const { std::vector<QString>res; res.push_back(s1); res.push_back(s2); res.push_back(s3); res.push_back(s4); return checkErrorMessageFor(cs,res); }
    bool checkErrorMessageFor( Qt::CaseSensitivity cs, const QString &s1, const QString &s2, const QString &s3, const QString &s4, const QString &s5 ) const { std::vector<QString>res; res.push_back(s1); res.push_back(s2); res.push_back(s3); res.push_back(s4); res.push_back(s5); return checkErrorMessageFor(cs,res); }

    /*
        erroneous           - ошибочный
        erroneously         - ошибочно 
        завершено с ошибкой - completed with an error
        ошибочно            - wrong

        isCompletionError -> isCompletedWithError

        Error transferring https://api-invest.tinkoff.ru/openapi/market/candles?figi=BBG000HLJ7M4&from=2016-01-03T00%3A00%3A00.000000
        {"trackingId":"541263ba6ef286a5","payload":{"message":"[to]: Bad candle interval: from=2016-01-04T00:00:00Z to=2018-01-08T00:00:00Z expected from 7 days to 2 years","code":"VALIDATION_ERROR"},"status":"Error"}B00%3A00&to=2018-01-03T00%3A00%3A00.000000{"trackingId":"541263ba6ef286a5","payload":{"message":"[to]: Bad candle interval: from=2016-01-04T00:00:00Z to=2018-01-08T00:00:00Z expected from 7 days to 2 years","code":"VALIDATION_ERROR"},"status":"Error"}B00%3A00&interval=week - server replied: Internal Server Error, {"trackingId":"541263ba6ef286a5","payload":{"message":"[to]: Bad candle interval: from=2016-01-04T00:00:00Z to=2018-01-08T00:00:00Z expected from 7 days to 2 years","code":"VALIDATION_ERROR"},"status":"Error"}

     */

protected:

    QString getErrorMessageInternal() const
    {
        return m_errorMessage;
    }


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

    

    virtual void onError   ( Empty                          v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }

    virtual void onError   ( SandboxRegisterResponse        v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }

    virtual void onError   ( MarketInstrumentListResponse   v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }

    virtual void onError   ( CandlesResponse                v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }
    virtual void onError   ( OrderbookResponse              v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }
    virtual void onError   ( SearchMarketInstrumentResponse v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }

    virtual void onError   ( UserAccountsResponse           v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }

    virtual void onError   ( OperationsResponse             v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }

    virtual void onError   ( OrdersResponse                 v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }
    virtual void onError   ( LimitOrderResponse             v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }
    virtual void onError   ( MarketOrderResponse            v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }

    virtual void onError   ( PortfolioCurrenciesResponse    v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }
    virtual void onError   ( PortfolioResponse              v, QNetworkReply::NetworkError et, QString es ) { errorComplete(et, es); }
    


}; // class OpenApiCompletableFutureBase


} // namespace invest_openapi


