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


#include "database_config.h"
#include "database_manager_defs.h"

//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
namespace invest_openapi
{



//----------------------------------------------------------------------------
struct IDatabaseManager
{
    enum class IfExists
    {
        ifAnyway     ,
        ifExists     ,
        ifNotExists  ,
    };

    virtual std::size_t getQueryResultSize( QSqlQuery &query, bool needBool = false ) const = 0;

    // test helpers

    virtual QString getTableExistString( const QString &tableName ) const = 0;

    // Common helpers
    virtual QString q( const QString &str ) const = 0;


    // Core functions

    virtual QString   tableMapName       ( const QString &tableName                   ) const = 0; // to internal name mapping
    virtual QString   tableGetShema      ( QString tableName                          ) const = 0;
    virtual bool      tableCheckExist    ( const QString &tableName                   ) const = 0;
    virtual bool      tableDrop          ( const QString &tableName, IfExists existence = IfExists::ifExists     ) const = 0;
    virtual bool      tableCreate        ( const QString &tableName, IfExists existence = IfExists::ifNotExists  ) const = 0;

    // Table creation helpers
    virtual void      setDefDecimal      ( unsigned total, unsigned frac )       = 0;
    virtual QString   defDecimal         ( ) const = 0;
    virtual QString   defDecimal         ( unsigned total, unsigned frac ) const = 0;


}; // struct IDatabaseManager

//----------------------------------------------------------------------------




} // namespace invest_openapi

