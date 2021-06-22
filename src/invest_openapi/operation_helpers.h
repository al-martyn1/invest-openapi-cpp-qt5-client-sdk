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
#include <iostream>
#include <iomanip>
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
    //QString operationStatusStr = op.getStatus().asJson().toUpper();
    //if (operationStatusStr!="DONE" && operationStatusStr!="PROGRESS") // Also may be DECLINE or INVALID_...
    auto statusValue = op.getStatus().getValue();
    if (statusValue==OpenAPI::OperationStatus::eOperationStatus::DONE || statusValue==OpenAPI::OperationStatus::eOperationStatus::PROGRESS) // INVALID_VALUE_OPENAPI_GENERATED / DECLINE
        return true;

    return false;

}

//----------------------------------------------------------------------------
inline
bool isOperationTypeAnyBuyOrSell( const OpenAPI::Operation &op )
{
    //QString operationTypeStr   = op.getOperationType().asJson().toUpper();
    //if (operationTypeStr!="BUYCARD" && operationTypeStr!="BUY" && operationTypeStr!="SELL")

    auto operationTypeValue = op.getOperationType().getValue(); // OperationTypeWithCommission::eOperationTypeWithCommission
    if ( operationTypeValue==OpenAPI::OperationTypeWithCommission::eOperationTypeWithCommission::BUY 
      || operationTypeValue==OpenAPI::OperationTypeWithCommission::eOperationTypeWithCommission::BUYCARD 
      || operationTypeValue==OpenAPI::OperationTypeWithCommission::eOperationTypeWithCommission::SELL
      )
        return true;

    return false;

}

//----------------------------------------------------------------------------
inline
bool isOperationTypeSell( const OpenAPI::Operation &op )
{
    // QString operationTypeStr   = op.getOperationType().asJson().toUpper();
    // if (operationTypeStr!="SELL")
    //     return false;
    // return true;

    return op.getOperationType().getValue() == OpenAPI::OperationTypeWithCommission::eOperationTypeWithCommission::SELL;

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
                                      , const QString                                                                                   &operationsMaxAge
                                      , std::map< QString, QSharedPointer< OpenApiCompletableFuture< OpenAPI::OperationsResponse > > >  &awaitingOperationResponses
                                      , std::map< QString, std::vector< OpenAPI::Operation > >                                          &completedOperationsByFigi
                                      , QString                                                                                         &statusStr
                                      , std::map< QString, std::set<QString> >                                                          *pFigiMismatches = 0 // for debug
                                      )
{

    bool noErrors = true;

    auto isFinished = []( auto it )
                      {
                          auto &f = it->second;
                          return f->isFinished();
                      };

    auto foundOpResponseIt = findOpenApiCompletableFutureFinished( awaitingOperationResponses.begin()
                                                                 , awaitingOperationResponses.end()
                                                                 , isFinished
                                                                 );
    // Iterator invalidation rules
    // https://stackoverflow.com/questions/6438086/iterator-invalidation-rules

    for( ; foundOpResponseIt!=awaitingOperationResponses.end()
         ; foundOpResponseIt = findOpenApiCompletableFutureFinished( foundOpResponseIt, awaitingOperationResponses.end(), isFinished )
       )
    {
        auto figiKey            = foundOpResponseIt->first;
        auto operationsResponse = foundOpResponseIt->second;

        //std::cout << "Found completed: " << figiKey << " : " << (void*)&(*operationsResponse) << std::endl;


        auto nextIt = foundOpResponseIt; ++nextIt;
        awaitingOperationResponses.erase(foundOpResponseIt);
        foundOpResponseIt = nextIt;

        if (!operationsResponse->isResultValid())
        {
            statusStr = QString("Get Operations Error: ") + operationsResponse->getErrorMessage();
            //throw std::runtime_error( statusStr.toStdString() );

            // Try to rerequest
            awaitingOperationResponses[figiKey] = pOpenApi->operations( operationsMaxAge, figiKey );

            //std::cout << "Got an error" << std::endl << std::endl;

            noErrors = false;
            //return false; // Stop on first error
            continue;
        }


        QList< OpenAPI::Operation > operations = operationsResponse->value.getPayload().getOperations();


        for( const auto &op : operations )
        {
            QString figi = op.getFigi().toUpper();

            if (figiKey!=figi)
            {
                //std::string msg = std::string("invest_openapi::processAwaitingOperationResponses: Something goes wrong: ") + figiKey.toStdString() + std::string("!=") + figi.toStdString();
                //throw std::runtime_error( msg );
                //std::cout << msg << std::endl << std::endl;

                if (pFigiMismatches)
                {
                    std::map< QString, std::set<QString> > &m = *pFigiMismatches;
                    m[figiKey].insert(figi);
                }
                
            }

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

    return noErrors;
}

//----------------------------------------------------------------------------
inline 
bool isOperationExecutedQauntityDiffers( const OpenAPI::Operation &op1, const OpenAPI::Operation &op2 )
{
    if (op1.getId()!=op2.getId())
        return true; // Ваще разные операции попались

    if (op1.getQuantityExecuted()!=op2.getQuantityExecuted())
        return true; // Операция та же, но число исполненых лотов другое

    return false; // Число исполненых лотов не изменилось
}

//----------------------------------------------------------------------------
//! Вектора должны быть отсортированы по дате. Это делается в processAwaitingOperationResponses, тут никак не проверяем
inline 
bool isOperationsOrExecutedQauntityDiffers( const std::vector< OpenAPI::Operation > &vec1, const std::vector< OpenAPI::Operation > &vec2 )
{
    if (vec1.size()!=vec2.size())
        return true; // Что-то, да поменялось

    std::vector< OpenAPI::Operation >::const_iterator it1 = vec1.begin(), it2 = vec2.begin();
    for(; it1!=vec1.end(); ++it1, ++it2)
    {
        if (isOperationExecutedQauntityDiffers( *it1, *it2 ))
            return true;
    }

    return false; // Вектора такие же, операции в них те же, и количество исполненых лотов не изменилось ни по одной из них
}

//----------------------------------------------------------------------------
//! Возвращает true, если состав и/или количество элементов хотя бы в одном эелементе mergeTo изменилось. По содержимому проверяем только id и количество executed лотов.
inline 
bool mergeOperationMaps( std::map< QString, std::vector< OpenAPI::Operation > > &mergeTo
                       , const std::map< QString, std::vector< OpenAPI::Operation > > &mergeFrom
                       )
{
    bool opsChanged = false;

    std::map< QString, std::vector< OpenAPI::Operation > >::const_iterator fit = mergeFrom.begin();
    for(; fit != mergeFrom.end(); ++fit )
    {
        std::map< QString, std::vector< OpenAPI::Operation > >::iterator tit = mergeTo.find(fit->first);
        if (tit==mergeTo.end())
        {
            opsChanged = true;
            mergeTo[fit->first] = fit->second;
            continue;
        }

        // Found
        if (!opsChanged)
        {
            opsChanged = isOperationsOrExecutedQauntityDiffers(tit->second, fit->second);
        }

        tit->second = fit->second;

    }

    return opsChanged;
}



//----------------------------------------------------------------------------




} // namespace invest_openapi

//----------------------------------------------------------------------------

