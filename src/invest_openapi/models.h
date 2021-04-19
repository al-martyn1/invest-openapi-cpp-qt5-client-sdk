/*! \file
    \brief 
 */

#pragma once


// Public Morozoff
// there is no legal way to access *_isValid field
//#define private public


#include "models/client/BrokerAccountType.h"
#include "models/client/Candle.h"
#include "models/client/CandleResolution.h"
#include "models/client/Candles.h"
#include "models/client/CandlesResponse.h"
#include "models/client/Currencies.h"
#include "models/client/Currency.h"
#include "models/client/CurrencyPosition.h"
#include "models/client/Empty.h"
#include "models/client/Enum.h"
#include "models/client/Error.h"
#include "models/client/Error_payload.h"
#include "models/client/Helpers.h"
#include "models/client/HttpFileElement.h"
#include "models/client/HttpRequest.h"
#include "models/client/InstrumentType.h"
#include "models/client/LimitOrderRequest.h"
#include "models/client/LimitOrderResponse.h"
#include "models/client/MarketApi.h"
#include "models/client/MarketInstrument.h"
#include "models/client/MarketInstrumentList.h"
#include "models/client/MarketInstrumentListResponse.h"
#include "models/client/MarketInstrumentResponse.h"
#include "models/client/MarketOrderRequest.h"
#include "models/client/MarketOrderResponse.h"
#include "models/client/MoneyAmount.h"
#include "models/client/Object.h"
#include "models/client/Operation.h"
#include "models/client/Operations.h"
#include "models/client/OperationsApi.h"
#include "models/client/OperationsResponse.h"
#include "models/client/OperationStatus.h"
#include "models/client/OperationTrade.h"
#include "models/client/OperationType.h"
#include "models/client/OperationTypeWithCommission.h"
#include "models/client/Order.h"
#include "models/client/Orderbook.h"
#include "models/client/OrderbookResponse.h"
#include "models/client/OrderResponse.h"
#include "models/client/OrdersApi.h"
#include "models/client/OrdersResponse.h"
#include "models/client/OrderStatus.h"
#include "models/client/OrderType.h"
#include "models/client/PlacedLimitOrder.h"
#include "models/client/PlacedMarketOrder.h"
#include "models/client/Portfolio.h"
#include "models/client/PortfolioApi.h"
#include "models/client/PortfolioCurrenciesResponse.h"
#include "models/client/PortfolioPosition.h"
#include "models/client/PortfolioResponse.h"
#include "models/client/SandboxAccount.h"
#include "models/client/SandboxApi.h"
#include "models/client/SandboxCurrency.h"
#include "models/client/SandboxRegisterRequest.h"
#include "models/client/SandboxRegisterResponse.h"
#include "models/client/SandboxSetCurrencyBalanceRequest.h"
#include "models/client/SandboxSetPositionBalanceRequest.h"
#include "models/client/SearchMarketInstrument.h"
#include "models/client/SearchMarketInstrumentResponse.h"
#include "models/client/TradeStatus.h"
#include "models/client/UserAccount.h"
#include "models/client/UserAccounts.h"
#include "models/client/UserAccountsResponse.h"
#include "models/client/UserApi.h"

#include "streaming_models/GenericStreamingResponse.h"
#include "streaming_models/StreamingError.h"
#include "streaming_models/StreamingOrderbook.h"
#include "streaming_models/StreamingOrderbookResponse.h"

#include "streaming_models/StreamingOrderbookItem.h"



// #undef private


namespace invest_openapi
{

    using namespace OpenAPI;

} // namespace invest_openapi

