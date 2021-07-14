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
inline
OpenAPI::MoneyAmount getPortfolioPositionSpentCost( const OpenAPI::PortfolioPosition &p )
{
    OpenAPI::MoneyAmount res = p.getAveragePositionPrice();

    marty::Decimal avgPrice   = res.getValue();
    marty::Decimal posBalance = p.getBalance();

    marty::Decimal cost = avgPrice * posBalance;

    /*
    qDebug().nospace().noquote() << "??? " << "getPortfolioPositionSpentCost for " << p.getTicker() << " - "
                                           << "Price: " << avgPrice.toString().c_str() << ", "
                                           << "Balance: " << posBalance.toString().c_str() << ", "
                                           << "Cost: " << cost.toString().c_str() ; // << ", "
    */

    //res.setValue( res.getValue() * p.getBalance() );
    res.setValue( cost );

    return res;
}

//----------------------------------------------------------------------------
inline
OpenAPI::MoneyAmount getPortfolioPositionCurrentCost( const OpenAPI::PortfolioPosition &p /* , bool excludeBrockerCredit = true */  )
{
    OpenAPI::MoneyAmount spentCost     = getPortfolioPositionSpentCost(p);

    OpenAPI::MoneyAmount expectedYield = p.getExpectedYield();

    OpenAPI::Currency scCurrency = spentCost.getCurrency();

    OpenAPI::Currency eyCurrency = expectedYield.getCurrency();

    if (scCurrency.asJson().toUpper()!=eyCurrency.asJson().toUpper())
    {
        qCritical().nospace().noquote() << "!!! Portolio position " << p.getTicker() << " (" << p.getFigi() << ") - "
                                        << "Balance currency (" << scCurrency.asJson() << ") is different than Eexpected Yield currency (" << eyCurrency.asJson() << ")";
    }

    marty::Decimal spent    = spentCost.getValue();
    marty::Decimal expected = expectedYield.getValue();

    // if (spent.sign()<0 && excludeBrockerCredit)
    //     spent = marty::Decimal(0);  // Эти деньги, условно говоря, взяты у брокера в долг, наша - только прибыль

    marty::Decimal summary  = spent + expected;

    /*
    qDebug().nospace().noquote() << "    " << "getPortfolioPositionCurrentCost for " << p.getTicker() << " - "
                                           << "Spent Cost: " << spent.toString().c_str() << ", "
                                           << "Expected Yield: " << expected.toString().c_str() << ", "
                                           << "Summary: " << summary.toString().c_str() ; // << ", "
    */

    spentCost.setValue( summary );

    return spentCost;
}

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

