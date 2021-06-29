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


// PortfolioResponse
// Portfolio getPayload() const;

// Portfolio
// QList<PortfolioPosition> getPositions() const;

// PortfolioPosition
// QString getFigi()

// marty::Decimal getBalance()
// marty::Decimal getBlocked()

// MoneyAmount getAveragePositionPrice()

// qint32 getLots()


// MoneyAmount
// Currency getCurrency()
// marty::Decimal getValue()


//----------------------------------------------------------------------------
//! 
inline
void parsePortfolio( const QList<PortfolioPosition>          &positions
                   , std::map< QString, PortfolioPosition >  &parseToMap
                   )
{
    for( const auto &p : positions )
    {
        //parseToMap[p.getFigi().toUpper()].push_back(p);
        parseToMap[p.getFigi().toUpper()] = p;
    }
}

//----------------------------------------------------------------------------
//! Return true if response is valid, and payload is also valid
inline
bool parsePortfolioResponse( const OpenAPI::PortfolioResponse        &portfolioResponse
                           , std::map< QString, PortfolioPosition >  &parseToMap
                           )
{
    if (!portfolioResponse.isSet() || !portfolioResponse.isValid())
        return false;

    if (!portfolioResponse.is_payload_Set() || !portfolioResponse.is_payload_Valid())
        return false;

    parsePortfolio( portfolioResponse.getPayload().getPositions()
                  , parseToMap
                  );

    return true;
}

//----------------------------------------------------------------------------
inline // return true if no error, response is valid and parsed ok
bool processCompletedPortfolioResponse( QSharedPointer< OpenApiCompletableFuture<OpenAPI::PortfolioResponse> >   portfolioResponse
                                      , std::map< QString, PortfolioPosition >                                  &parseToMap
                                   )
{
    if (!portfolioResponse)
        throw("invest_openapi::processCompletedPortfolioResponse: portfolioResponse is zero");

    //if (!ordersResponse->isFinished())
    //    return false;

    if (!portfolioResponse->isResultValid())
        return false;

    return parsePortfolioResponse( portfolioResponse->result(), parseToMap );

}

//----------------------------------------------------------------------------






// PortfolioCurrenciesResponse
// Currencies getPayload() const;

// Currencies
// QList<CurrencyPosition> getCurrencies() const;

// CurrencyPosition
// Currency getCurrency() const;
// marty::Decimal getBalance() const;
// marty::Decimal getBlocked() const;

// Currency
// QString asJson() const override;
// Currency::eCurrency getValue() const;

//----------------------------------------------------------------------------
//! 
inline
void parsePortfolioCurrencies( const QList<CurrencyPosition>          &positions
                             , std::map< QString, CurrencyPosition >  &parseToMap
                             )
{
    for( const auto &p : positions )
    {
        parseToMap[p.getCurrency().asJson().toUpper()] = p;
    }
}

//----------------------------------------------------------------------------
//! Return true if response is valid, and payload is also valid
inline
bool parsePortfolioCurrenciesResponse( const OpenAPI::PortfolioCurrenciesResponse  &portfolioCurrenciesResponse
                                     , std::map< QString, CurrencyPosition >       &parseToMap
                                     )
{
    if (!portfolioCurrenciesResponse.isSet() || !portfolioCurrenciesResponse.isValid())
        return false;

    if (!portfolioCurrenciesResponse.is_payload_Set() || !portfolioCurrenciesResponse.is_payload_Valid())
        return false;

    parsePortfolioCurrencies( portfolioCurrenciesResponse.getPayload().getCurrencies()
                            , parseToMap
                            );

    return true;
}

//----------------------------------------------------------------------------
inline // return true if no error, response is valid and parsed ok
bool processCompletedPortfolioCurrenciesResponse( QSharedPointer< OpenApiCompletableFuture<OpenAPI::PortfolioCurrenciesResponse> >  portfolioCurrenciesResponse
                                                , std::map< QString, CurrencyPosition >                                            &parseToMap
                                                )
{
    if (!portfolioCurrenciesResponse)
        throw("invest_openapi::processCompletedPortfolioCurrenciesResponse: portfolioCurrenciesResponse is zero");

    //if (!ordersResponse->isFinished())
    //    return false;

    if (!portfolioCurrenciesResponse->isResultValid())
        return false;

    return parsePortfolioCurrenciesResponse( portfolioCurrenciesResponse->result(), parseToMap );

}

//----------------------------------------------------------------------------




} // namespace invest_openapi

//----------------------------------------------------------------------------

