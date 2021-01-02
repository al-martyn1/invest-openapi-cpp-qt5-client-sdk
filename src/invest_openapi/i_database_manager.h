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


#include "utility.h"
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
    virtual void      setDefDecimal      ( unsigned total, unsigned frac )       = 0;
    virtual QString   defDecimal         ( ) const = 0;
    virtual QString   defDecimal         ( unsigned total, unsigned frac ) const = 0;

    virtual QString q( const QString &str  ) const = 0;
    virtual QString lf( char comma = ' ' ) const = 0;
    virtual QString tab() const = 0;
    virtual QString escape( const QString &str ) const = 0;

    // Core functions

    virtual QString     tableMapName     ( const QString &tableName                   ) const = 0; // to internal name mapping
    virtual QString     tableGetShema    ( const QString &tableName                   ) const = 0;
    virtual QSqlQuery   selectExecHelper ( const QString &queryText                   ) const = 0;

    virtual QVector<QString> queryToSingleStringVector( QSqlQuery& query, int valIdx, const QVector<QString> &except, bool caseCompare ) const = 0;
    virtual QVector<QString> queryToSingleStringVector( QSqlQuery& query, int valIdx, const QStringList      &except, bool caseCompare ) const = 0;
    virtual QVector<QString> queryToSingleStringVector( QSqlQuery& query, int valIdx,       QString           except, bool caseCompare ) const = 0;
    virtual QVector<QString> queryToSingleStringVector( QSqlQuery& query, int valIdx                                                   ) const = 0;

    virtual QVector<QString> tableGetNames    () const = 0;
    virtual QVector<QString> tableGetColumnsInternal ( const QString &internalTableName ) const = 0;
    virtual QVector<QString> tableGetColumns  ( const QString &tableName ) const = 0;


    // Meta helpers
    virtual bool      metaInsertForTableExact  ( QString tableName                    , QString tableDisplayName , QString tableDescription ) const = 0;
    virtual bool      metaInsertForColumnExact ( QString tableName, QString columnName, QString columnDisplayName, QString tableDescription ) const = 0;

    virtual bool      metaInsertForTable       ( QStringTriple tripple ) const = 0;
    virtual bool      metaInsertForColumn      ( QStringQuatro quatro  ) const = 0;

    virtual bool      metaInsertForTableBulk   ( const QString &bulkText ) const = 0;
    virtual bool      metaInsertForColumnBulk  ( const QString &bulkText ) const = 0;

    // Table creation helpers
    virtual bool      tableCheckExist    ( const QString &tableName                                              ) const = 0;
    virtual bool      tableDrop          ( const QString &tableName, IfExists existence = IfExists::ifExists     ) const = 0;
    virtual bool      tableCreate        ( const QString &tableName, IfExists existence = IfExists::ifNotExists  ) const = 0;


}; // struct IDatabaseManager

//----------------------------------------------------------------------------




} // namespace invest_openapi

