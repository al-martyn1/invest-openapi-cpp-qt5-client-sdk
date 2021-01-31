#pragma once

#include <QObject>
#include <QVariant>
#include <QString>
#include <QVector>
#include <QSet>
#include <QMap>
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
    virtual void      applyDefDecimalFromConfig( const DatabaseConfig & cfg )    = 0;

    virtual QString   defDecimal         ( ) const = 0;
    virtual QString   defDecimal         ( unsigned total, unsigned frac ) const = 0;

    virtual QString q( const QString &str  ) const = 0;
    virtual QString lf( char comma = ' ' ) const = 0;
    virtual QString tab() const = 0;
    virtual QString sqlQuote( const QString &str ) const = 0;
    virtual QString sqlQuote( const QVariant &str ) const = 0;
    virtual QVector<QString> sqlQuote( const QVector<QString > &strs ) const = 0;
    virtual QVector<QString> sqlQuote( const QVector<QVariant> &strs ) const = 0;

    // Core functions

    virtual QSet<QString> tableGetTableNamesForCreation( int creationLevel ) const = 0;

    virtual QString     tableMapName     ( const QString &tableName                   ) const = 0; // to internal name mapping
    virtual QString     tableGetShema    ( const QString &tableName                   ) const = 0;
    virtual QSqlQuery   selectExecHelper ( const QString &queryText                   ) const = 0;

    virtual QVector<QString> tableGetNames    () const = 0;
    virtual QVector<QString> tableGetColumnsInternal ( const QString &internalTableName ) const = 0;
    virtual QVector<QString> tableGetColumns  ( const QString &tableName ) const = 0;

    virtual QVector<QString> queryToSingleStringVector( QSqlQuery& query, int valIdx, const QVector<QString> &except, bool caseCompare ) const = 0;
    virtual QVector<QString> queryToSingleStringVector( QSqlQuery& query, int valIdx, const QStringList      &except, bool caseCompare ) const = 0;
    virtual QVector<QString> queryToSingleStringVector( QSqlQuery& query, int valIdx,       QString           except, bool caseCompare ) const = 0;
    virtual QVector<QString> queryToSingleStringVector( QSqlQuery& query, int valIdx                                                   ) const = 0;


    virtual bool insertToImpl( const QString &tableName, const QVector< QVector<QString> > &valsVecVec, const QVector<QString> &tableColumnNames ) const = 0;

    /*
    virtual bool insertTo( const QString &tableName, const QVector<QString> &valsVec, const QVector<QString> &tableColumnNames ) const
    {
        QVector< QVector<QString> > valsVecVec;
        valsVecVec.push_back( valsVec );
        return insertToImpl( tableName, valsVecVec, tableColumnNames );
    }
    */

    #define INVEST_OPENAPI_IDATABASEMANAGER_INSERTTO_DECLARE_WITH_DEF_IMPLEMENTATION( valsListType, tableColumnNamesListType )                   \
    virtual bool insertTo( const QString &tableName, const valsListType &valsList, const tableColumnNamesListType &tableColumnNamesList) const   \
    {                                                                                                                                            \
        return insertToImpl( tableName, convertToQVectorOfQVectorOfQStrings(valsList), convertToQVectorOfQStrings(tableColumnNamesList) );           \
    }

    INVEST_OPENAPI_IDATABASEMANAGER_INSERTTO_DECLARE_WITH_DEF_IMPLEMENTATION( QVector<QVector<QVariant > >, QVector<QString> )
    INVEST_OPENAPI_IDATABASEMANAGER_INSERTTO_DECLARE_WITH_DEF_IMPLEMENTATION( QVector<QVector<QString  > >, QVector<QString> )
    INVEST_OPENAPI_IDATABASEMANAGER_INSERTTO_DECLARE_WITH_DEF_IMPLEMENTATION( QVector<QStringList        >, QVector<QString> )
    INVEST_OPENAPI_IDATABASEMANAGER_INSERTTO_DECLARE_WITH_DEF_IMPLEMENTATION( QVector<QString            >, QVector<QString> )

    INVEST_OPENAPI_IDATABASEMANAGER_INSERTTO_DECLARE_WITH_DEF_IMPLEMENTATION( QVector<QVector<QVariant > >, QStringList )
    INVEST_OPENAPI_IDATABASEMANAGER_INSERTTO_DECLARE_WITH_DEF_IMPLEMENTATION( QVector<QVector<QString  > >, QStringList )
    INVEST_OPENAPI_IDATABASEMANAGER_INSERTTO_DECLARE_WITH_DEF_IMPLEMENTATION( QVector<QStringList        >, QStringList )
    INVEST_OPENAPI_IDATABASEMANAGER_INSERTTO_DECLARE_WITH_DEF_IMPLEMENTATION( QVector<QString            >, QStringList )

    INVEST_OPENAPI_IDATABASEMANAGER_INSERTTO_DECLARE_WITH_DEF_IMPLEMENTATION( QVector<QVector<QVariant > >, QString )
    INVEST_OPENAPI_IDATABASEMANAGER_INSERTTO_DECLARE_WITH_DEF_IMPLEMENTATION( QVector<QVector<QString  > >, QString )
    INVEST_OPENAPI_IDATABASEMANAGER_INSERTTO_DECLARE_WITH_DEF_IMPLEMENTATION( QVector<QStringList        >, QString )
    INVEST_OPENAPI_IDATABASEMANAGER_INSERTTO_DECLARE_WITH_DEF_IMPLEMENTATION( QVector<QString            >, QString )


    virtual bool insertToBulkFromString( const QString &tableName, const QString &vals, const QVector<QString> &tableColumnNames ) const
    {
         return insertTo( tableName, listStringSplit(vals.split( ';', Qt::SkipEmptyParts )), tableColumnNames );
         
    }

    // Meta helpers

    //! insert meta for all tables on single call
    virtual bool      metaInsertForTablesBulk ( const QString &bulkText ) const = 0;
    //! insert meta for all columns of single table on single call
    virtual bool      metaInsertForColumnBulk ( QString tableName, const QString &bulkText ) const = 0;

    // Table creation helpers
    virtual bool      tableCheckExist    ( const QString &tableName                                              ) const = 0;
    virtual bool      tableDrop          ( const QString &tableName, IfExists existence = IfExists::ifExists     ) const = 0;
    virtual bool      tableCreate        ( const QString &tableName, IfExists existence = IfExists::ifNotExists  ) const = 0;


}; // struct IDatabaseManager

//----------------------------------------------------------------------------




} // namespace invest_openapi

