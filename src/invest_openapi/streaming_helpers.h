/*! \file
    \brief 
 */

#pragma once

#include <QObject>
#include <QDebug>
#include <QtDebug>
#include <QTest>
#include <QTimeZone>

//#include <QtConcurrent/QtConcurrent>

#include <iostream>
#include <exception>
#include <stdexcept>
#include <memory>
#include <map>

#include "models.h"

#include "invest_openapi_safe_main.h"
#include "openapi_completable_future_base.h"
#include "openapi_completable_future.h"
#include "api_config.h"
#include "auth_config.h"
#include "logging_config.h"
#include "currencies_config.h"
#include "factory.h"
#include "utility.h"
#include "qdebug_support.h"
#include "marty_decimal.h"
#include "qt_time_helpers.h"



//----------------------------------------------------------------------------
namespace invest_openapi
{



//----------------------------------------------------------------------------
template < typename ConnectedHandler
         , typename DisconnectedHandler
         , typename DataHandler
         > inline
void connectStreamingWebSocket( QSharedPointer<IOpenApi>              pOpenApi
                              , QWebSocket                           &webSocket
                              , ConnectedHandler                      onConnected
                              , DisconnectedHandler                   onDisconnected
                              , DataHandler                           onStreamingMessage
                              )
{
    webSocket.connect( &webSocket, &QWebSocket::connected           , onConnected        );
    webSocket.connect( &webSocket, &QWebSocket::disconnected        , onDisconnected     );
    webSocket.connect( &webSocket, &QWebSocket::textMessageReceived , onStreamingMessage );

    webSocket.open( pOpenApi->getStreamingApiNetworkRequest() );

}







//----------------------------------------------------------------------------




//----------------------------------------------------------------------------


} // namespace invest_openapi

