#pragma once




#include <QCoreApplication>
#include <QString>
#include <QSettings>
#include <QTest>
#include <QDir>
#include <QElapsedTimer>
#include <QtWebSockets>

#include <algorithm>
#include <iterator>
#include <ostream>

#include "models.h"
#include "invest_openapi_types.h"




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
template< typename StreamType, typename SpaceType, typename EndlType, typename OpsIterator > inline
void operationPrintBrief( StreamType &os, SpaceType indend, const EndlType &endlType, OpsIterator b, OpsIterator e )
{
    for(; b!e; ++b)
    {
        os << indend << "Operation: " << b->getOperationType().asJson().toUpper() << endlType(os);
        os << indend << "Status   : " << b->getStatus().asJson().toUpper()        << endlType(os);
        os << indend << "Date&Time: " << b->getDate()                             << endlType(os);
        os << indend << "Trades # : " << b->getTrades().size()                    << endlType(os);
        os << endlType(os);
    
    }

    os << endlType(os);

}







//----------------------------------------------------------------------------




} // namespace invest_openapi

//----------------------------------------------------------------------------

