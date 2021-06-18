#pragma once




#include <QCoreApplication>
#include <QString>
#include <QSettings>
#include <QTest>
#include <QDir>
#include <QList>
#include <QElapsedTimer>
#include <QtWebSockets>

#include <algorithm>
#include <iterator>
#include <ostream>
#include <exception>
#include <stdexcept>

#include "models.h"
#include "invest_openapi_types.h"
#include "invest_openapi.h"
#include "ioa_ostream.h"
#include "openapi_completable_future.h"
#include "openapi_limits.h"
#include "cpp_helpers.h"


//----------------------------------------------------------------------------
namespace invest_openapi
{



//----------------------------------------------------------------------------
inline
bool isOperationStatusDoneOrInProgress( const OpenAPI::Operation &op )
{
    QString operationStatusStr = op.getStatus().asJson().toUpper();
    if (operationStatusStr!="DONE" && operationStatusStr!="PROGRESS") // Also may be DECLINE or INVALID_...
        return false;

    return true;

}

//----------------------------------------------------------------------------
inline
bool isOperationTypeAnyBuyOrSell( const OpenAPI::Operation &op )
{
    QString operationTypeStr   = op.getOperationType().asJson().toUpper();
    if (operationTypeStr!="BUYCARD" && operationTypeStr!="BUY" && operationTypeStr!="SELL")
        return false;

    return true;

}

//----------------------------------------------------------------------------
inline
bool isOperationTypeSell( const OpenAPI::Operation &op )
{
    QString operationTypeStr   = op.getOperationType().asJson().toUpper();
    if (operationTypeStr!="SELL")
        return false;

    return true;

}

//----------------------------------------------------------------------------
inline
void sortOperationsByDateAscending( std::vector< OpenAPI::Operation > & ops )
{
    std::stable_sort( ops.begin(), ops.end()
                    , []( const OpenAPI::Operation &op1, const OpenAPI::Operation &op2 )
                      {
                          return op1.getDate() < op2.getDate();
                      }
                    );
}

//----------------------------------------------------------------------------
inline
void sortOperationsByDateDescending( std::vector< OpenAPI::Operation > & ops )
{
    std::stable_sort( ops.begin(), ops.end()
                    , []( const OpenAPI::Operation &op1, const OpenAPI::Operation &op2 )
                      {
                          return op1.getDate() > op2.getDate();
                      }
                    );
}

//----------------------------------------------------------------------------
inline
void sortOperationsByDate( std::vector< OpenAPI::Operation > & ops, SortType sortType )
{
    if (sortType==SortType::ascending)
        sortOperationsByDateAscending(ops);
    else
        sortOperationsByDateDescending(ops);
}

//----------------------------------------------------------------------------
template< typename SrcIteratorType, typename InserterType > inline
void getSomeOfFirstOperations( std::size_t numOpsToGet
                             , SrcIteratorType b, SrcIteratorType e
                             , InserterType inserterSell, InserterType inserterBuy
                             )
{
    std::size_t gottenSells = 0, gottenBuys = 0;

    for( ; b!=e; ++b )
    {
        if (isOperationTypeSell( *b ) )
        {
            if (gottenSells<numOpsToGet)
            {
                *inserterSell++ = *b;
                ++gottenSells;
            }
        }
        else
        {
            if (gottenBuys<numOpsToGet)
            {
                *inserterBuy++ = *b;
                ++gottenBuys;
            }
        }
    }
}

//----------------------------------------------------------------------------
template< typename VectorType > inline
void getSomeOfFirstOperations( std::size_t numOpsToGet
                             , const VectorType   &src
                             , VectorType &sells, VectorType &buys
                             , bool bAppend = false
                             )
{
    if (!bAppend)
    {
        sells.clear();
        buys .clear();
    }

    getSomeOfFirstOperations( numOpsToGet, src.begin(), src.end(), std::back_inserter(sells), std::back_inserter(buys) );
}

//----------------------------------------------------------------------------
template< typename StreamType, typename SpaceType, typename OpsIterator > inline
void operationPrintBrief( StreamType &os, SpaceType indend, OpsIterator b, OpsIterator e )
{
    for(; b!=e; ++b)
    {
        const auto &op = *b;
        os << indend << "Operation: " << op.getOperationType().asJson().toUpper() << "\n";
        os << indend << "Status   : " << op.getStatus().asJson().toUpper()        << "\n";
        os << indend << "Date&Time: " << op.getDate()                             << "\n";
        os << indend << "Trades # : " << op.getTrades().size()                    << "\n";
        os << "\n";
    
    }

    os << "\n";

}

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
inline // return true if no error
bool processAwaitingOperationResponses( QSharedPointer<IOpenApi> pOpenApi
                                      , std::vector< QSharedPointer< OpenApiCompletableFuture< OpenAPI::OperationsResponse > > > &awaitingOperationResponses
                                      , std::set< QString >                                                                      &awaitingOperationResponsesFigiSet
                                      , std::map< QString, std::vector< OpenAPI::Operation > >                                   &completedOperationsByFigi
                                      , QString                                                                                  &statusStr
                                      )
{
    if (awaitingOperationResponses.empty())
    {
        QElapsedTimer operationsRequestTimer;
        operationsRequestTimer.start();
       
        std::size_t requestCounter = 0;

        std::set< QString >::const_iterator figiIt = awaitingOperationResponsesFigiSet.begin();

        for(; figiIt != awaitingOperationResponsesFigiSet.end(); ++figiIt, ++requestCounter )
        {
            checkWaitOnRequestsLimit( operationsRequestTimer, requestCounter );

            QString requestForFigi   = *figiIt;
            auto operationsResponse  = pOpenApi->operations( "10YEAR", requestForFigi);
            awaitingOperationResponses.push_back(operationsResponse);
        }

    }


    auto foundOpResponseIt = findOpenApiCompletableFutureFinished( awaitingOperationResponses.begin(), awaitingOperationResponses.end() );
    //if (foundOpResponseIt==awaitingOperationResponses.end())
    //    return;

    for( ; foundOpResponseIt!=awaitingOperationResponses.end()
         ; foundOpResponseIt = findOpenApiCompletableFutureFinished( awaitingOperationResponses.begin(), awaitingOperationResponses.end() )
       )
    {
        auto operationsResponse = *foundOpResponseIt;

        awaitingOperationResponses.erase(foundOpResponseIt);

        if (!operationsResponse->isResultValid())
        {
            statusStr = QString("Get Operations Error: ") + operationsResponse->getErrorMessage();
            throw std::runtime_error( statusStr.toStdString() );
            return false; // Stop on first error
        }


        QList< OpenAPI::Operation > operations = operationsResponse->value.getPayload().getOperations();


        for( const auto &op : operations )
        {
            QString figi = op.getFigi().toUpper();
            cpp_helpers::conainerKeyErase(awaitingOperationResponsesFigiSet, figi);

            if ( !isOperationStatusDoneOrInProgress(op) )
                continue; // May be DECLINE or INVALID_...

            if ( !isOperationTypeAnyBuyOrSell(op) ) // Also may be BUYCARD
                continue;

            completedOperationsByFigi[figi].push_back(op);
        }

    }



    std::map< QString, std::vector< OpenAPI::Operation > >::iterator it = completedOperationsByFigi.begin();
    for( ; it!=completedOperationsByFigi.end(); ++it )
    {
        const QString &figi = it->first;
        auto &opVec = it->second;

        sortOperationsByDate( opVec, SortType::descending );
    }

    return true;
}





//----------------------------------------------------------------------------




} // namespace invest_openapi

//----------------------------------------------------------------------------

