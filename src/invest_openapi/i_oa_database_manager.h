#pragma once

#include <QObject>
#include <QVariant>
#include <QString>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlField>
#include <QSqlQuery>
#include <QSharedPointer>

#include <exception>
#include <stdexcept>



//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
namespace invest_openapi
{




//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
struct IOaDatabaseManager
{

    virtual bool   insertNewCurrencyType     ( Currency         c, const QString & description ) const = 0;
    virtual bool   insertNewInstrumentType   ( InstrumentType   t, const QString & description ) const = 0;

    virtual bool   insertNewCurrencyType     ( const QString   &c, const QString & description ) const = 0;
    virtual bool   insertNewInstrumentType   ( const QString   &t, const QString & description ) const = 0;

    virtual bool   insertNewCurrencyTypes    ( const QString &all ) const = 0;
    virtual bool   insertNewInstrumentTypes  ( const QString &all ) const = 0;

}; // struct IOaDatabaseManager

//----------------------------------------------------------------------------




} // namespace invest_openapi

