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
#include <map>
#include <vector>


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
    virtual void      setDefaultDecimalFormat     ( unsigned total, unsigned frac )    = 0;
    virtual void      applyDefDecimalFormatFromConfig( const DatabaseConfig & cfg )    = 0;

    virtual QString   defDecimal         ( ) const = 0;
    virtual QString   defDecimal         ( unsigned total, unsigned frac ) const = 0;

    virtual QString q( const QString &str  ) const = 0;
    virtual QString lf( char comma = ' ' ) const = 0;
    virtual QString tab() const = 0;
    virtual QString sqlQuote( const QString  &str     ) const = 0;
    virtual QString sqlQuote( const QVariant &variant ) const = 0;
    virtual QString sqlQuote( int             i       ) const = 0;
    virtual QString sqlQuote( unsigned        u       ) const = 0;
    virtual QVector<QString> sqlQuote( const QVector<QString > &strs     ) const = 0;
    virtual QVector<QString> sqlQuote( const QVector<QVariant> &variants ) const = 0;
    virtual QVector<QString> sqlQuote( const QVector<int>      &ints     ) const = 0;
    virtual QVector<QString> sqlQuote( const QVector<unsigned> &uints    ) const = 0;


    // Core functions

    virtual QSet<QString> tableGetTableNamesForCreation( int creationLevel ) const = 0;

    virtual QString     tableMapName     ( const QString &tableName                   ) const = 0; // to internal name mapping
    virtual QString     tableGetSchema   ( const QString &tableName                   ) const = 0;
    virtual QVector<QString> tableGetAdditionalUniques  ( const QString &tableName    ) const = 0;
    virtual QVector<QString> tableGetColumnsFromSchema  ( const QString &tableName ) const = 0;

    virtual QSqlQuery   execHelper ( const QString &queryText, bool *pRes = 0 ) const = 0;

    virtual QVector< QString> execSelectQueryReturnFirstRow( const QString &queryText ) const = 0;


    virtual QString     makeSimpleSelectQueryText( const QString &tableName
                                                 , const QVector<QString > &fields = QVector<QString >() ) const = 0;

    virtual QString     makeSimpleSelectQueryText( const QString &tableName
                                                 , const QStringList &fields ) const
    {
        return makeSimpleSelectQueryText( tableName, convertToQVectorOfQStrings(fields) );
    }

    virtual QString     makeSimpleSelectQueryText( const QString &tableName
                                                 , const QString &fields ) const
    {
        return makeSimpleSelectQueryText( tableName, convertToQVectorOfQStrings(fields) );
    }


    virtual QString     makeSimpleSelectQueryText( const QString &tableName
                                                 , const QVector<QString> &whereNames, const QVector<QString> &whereVals
                                                 , const QVector<QString > &fields = QVector<QString >() ) const = 0;


    virtual QString     makeSimpleSelectQueryText( const QString &tableName
                                                 , const QVector<QString>  &whereNames, const QVector<QString> &whereVals
                                                 , const QStringList       &fields ) const
    {
        return makeSimpleSelectQueryText(tableName, whereNames, whereVals, convertToQVectorOfQStrings(fields) );
    }

    virtual QString     makeSimpleSelectQueryText( const QString &tableName
                                                 , const QVector<QString>  &whereNames, const QVector<QString> &whereVals
                                                 , const QString           &fields ) const
    {
        return makeSimpleSelectQueryText(tableName, whereNames, whereVals, convertToQVectorOfQStrings(fields) );
    }

    virtual QString     makeSimpleSelectQueryText( const QString     &tableName
                                                 , const QStringList &whereNames, const QStringList &whereVals
                                                 , const QVector<QString> &fields = QVector<QString >() ) const
    {
        return makeSimpleSelectQueryText(tableName, convertToQVectorOfQStrings(whereNames), convertToQVectorOfQStrings(whereVals), fields );
    }

    virtual QString     makeSimpleSelectQueryText( const QString     &tableName
                                                 , const QStringList &whereNames, const QStringList &whereVals
                                                 , const QStringList &fields ) const
    {
        return makeSimpleSelectQueryText(tableName, convertToQVectorOfQStrings(whereNames), convertToQVectorOfQStrings(whereVals), convertToQVectorOfQStrings(fields) );
    }

    virtual QString     makeSimpleSelectQueryText( const QString &tableName
                                                 , const QString &whereNames, const QString &whereVals
                                                 , const QVector<QString> &fields = QVector<QString >() ) const
    {
        return makeSimpleSelectQueryText(tableName, convertToQVectorOfQStrings(whereNames), convertToQVectorOfQStrings(whereVals), fields );
    }

    virtual QString     makeSimpleSelectQueryText( const QString &tableName
                                                 , const QString &whereNames, const QString &whereVals
                                                 , const QString &fields ) const
    {
        return makeSimpleSelectQueryText(tableName, convertToQVectorOfQStrings(whereNames), convertToQVectorOfQStrings(whereVals), convertToQVectorOfQStrings(fields) );
    }
/*
    virtual QString     makeSimpleSelectQueryText( const QString &tableName
                                                 , const char    *whereNames, const QString &whereVals
                                                 , const QString &fields ) const
    {
        return makeSimpleSelectQueryText(tableName, convertToQVectorOfQStrings(QString(whereNames)), convertToQVectorOfQStrings(whereVals), convertToQVectorOfQStrings(fields) );
    }
*/



    virtual QString     makeSimpleUpdateQueryText( const QString &tableName, const QString &whereNames, const QString &whereVals, const QVector<QString > &values, QVector<QString >  fields = QVector<QString >() ) const = 0;
    virtual QString     makeSimpleUpdateQueryText( const QString &tableName, const QString &whereNames, const QString &whereVals, const QVector<QString > &values, const QStringList &fields ) const
    {
        return makeSimpleUpdateQueryText( tableName, whereNames, whereVals, values, convertToQVectorOfQStrings(fields)  );
    }
    virtual QString     makeSimpleUpdateQueryText( const QString &tableName, const QString &whereNames, const QString &whereVals, const QVector<QString > &values, const QString     &fields ) const
    {
        return makeSimpleUpdateQueryText( tableName, whereNames, whereVals, values, convertToQVectorOfQStrings(fields)  );
    }

    virtual QString     makeSelectSingleValueQuery( const QString &queryText, const QString &valueField, bool fLast /* true for last, false for first */ ) const = 0;


    virtual QVector<QString> tableGetNamesFromDb    () const = 0;
    virtual QVector<QString> tableGetColumnsFromDbInternal ( const QString &internalTableName ) const = 0;
    virtual QVector<QString> tableGetColumnsFromDb  ( const QString &tableName ) const = 0;

    virtual QVector<QString> queryToSingleStringVector( QSqlQuery& query, int valIdx, const QVector<QString> &except, bool caseCompare ) const = 0;
    virtual QVector<QString> queryToSingleStringVector( QSqlQuery& query, int valIdx, const QStringList      &except, bool caseCompare ) const = 0;
    virtual QVector<QString> queryToSingleStringVector( QSqlQuery& query, int valIdx,       QString           except, bool caseCompare ) const = 0;
    virtual QVector<QString> queryToSingleStringVector( QSqlQuery& query, int valIdx                                                   ) const = 0;

    virtual QVector< QVector<QString> > selectResultToStringVectors( QSqlQuery& query ) const = 0;
    virtual QVector<QString>            selectFirstResultToSingleStringVector( QSqlQuery& query ) const = 0;

    virtual bool insertToImpl( const QString &tableName, const QVector< QVector<QString> > &valsVecVec, QVector<QString> tableColumnNames ) const = 0;

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
    INVEST_OPENAPI_IDATABASEMANAGER_INSERTTO_DECLARE_WITH_DEF_IMPLEMENTATION( QVector<QVector<QString  > >, QVector<QString> )    // !!!
    INVEST_OPENAPI_IDATABASEMANAGER_INSERTTO_DECLARE_WITH_DEF_IMPLEMENTATION( QVector<QStringList        >, QVector<QString> )
    //INVEST_OPENAPI_IDATABASEMANAGER_INSERTTO_DECLARE_WITH_DEF_IMPLEMENTATION( QVector<QString            >, QVector<QString> )

    INVEST_OPENAPI_IDATABASEMANAGER_INSERTTO_DECLARE_WITH_DEF_IMPLEMENTATION( QVector<QVector<QVariant > >, QStringList )
    INVEST_OPENAPI_IDATABASEMANAGER_INSERTTO_DECLARE_WITH_DEF_IMPLEMENTATION( QVector<QVector<QString  > >, QStringList )
    INVEST_OPENAPI_IDATABASEMANAGER_INSERTTO_DECLARE_WITH_DEF_IMPLEMENTATION( QVector<QStringList        >, QStringList )
    //INVEST_OPENAPI_IDATABASEMANAGER_INSERTTO_DECLARE_WITH_DEF_IMPLEMENTATION( QVector<QString            >, QStringList )

    INVEST_OPENAPI_IDATABASEMANAGER_INSERTTO_DECLARE_WITH_DEF_IMPLEMENTATION( QVector<QVector<QVariant > >, QString )
    INVEST_OPENAPI_IDATABASEMANAGER_INSERTTO_DECLARE_WITH_DEF_IMPLEMENTATION( QVector<QVector<QString  > >, QString )
    INVEST_OPENAPI_IDATABASEMANAGER_INSERTTO_DECLARE_WITH_DEF_IMPLEMENTATION( QVector<QStringList        >, QString )
    //INVEST_OPENAPI_IDATABASEMANAGER_INSERTTO_DECLARE_WITH_DEF_IMPLEMENTATION( QVector<QString            >, QString )

    /*
    virtual bool insertTo( const QString &tableName, const QVector<QVector<QString> > &valsList, const QVector<QString> &tableColumnNamesList) const
    {
        return insertToImpl( tableName, valsList, tableColumnNamesList );
    }

    virtual bool insertTo( const QString &tableName, const QVector<QStringList> &valsList, const QVector<QString> &tableColumnNamesList) const
    {
        return insertToImpl( tableName, convertToQVectorOfQVectorOfQStrings(valsList), tableColumnNamesList );
    }
    */


    virtual bool insertToBulkFromString( const QString &tableName, const QString &vals, const QVector<QString> &tableColumnNames = QVector<QString>() ) const
    {
         if (tableColumnNames.empty())
             return insertTo( tableName, listStringSplit(vals.split( ';', Qt::KeepEmptyParts )), tableGetColumnsFromSchema(tableName) );
         else
             return insertTo( tableName, listStringSplit(vals.split( ';', Qt::KeepEmptyParts )), tableColumnNames );
    }

    virtual bool insertToBulkFromString( const QString &tableName, const QString &vals, const QStringList &tableColumnNames ) const
    {
         if (tableColumnNames.isEmpty())
             return insertTo( tableName, listStringSplit(vals.split( ';', Qt::KeepEmptyParts )), tableGetColumnsFromSchema(tableName) );
         else
             return insertTo( tableName, listStringSplit(vals.split( ';', Qt::KeepEmptyParts )), convertToQVectorOfQStrings(tableColumnNames) );
    }

    virtual bool insertToBulkFromString( const QString &tableName, const QString &vals, const QString &tableColumnNames ) const
    {
         if (tableColumnNames.isEmpty())
             return insertTo( tableName, listStringSplit(vals.split( ';', Qt::KeepEmptyParts )), tableGetColumnsFromSchema(tableName) );
         else
             return insertTo( tableName, listStringSplit(vals.split( ';', Qt::KeepEmptyParts )), convertToQVectorOfQStrings(tableColumnNames) );
    }

    virtual QString getTableInitData( const QString &tableName ) const = 0;

    virtual void initTablesWithPredefinedData( const QStringList &tables, bool throwIfTableUnknown, bool throwIfInsertFailed ) const = 0;

    // Meta helpers

    //! insert meta for all tables on single call
    virtual bool      metaInsertForTablesBulk ( const QString &bulkText ) const = 0;
    //! insert meta for all columns of single table on single call
    virtual bool      metaInsertForColumnBulk ( QString tableName, const QString &bulkText ) const = 0;

    // Table creation helpers
    virtual bool      tableCheckExist    ( const QString &tableName                                              ) const = 0;
    virtual bool      tableDrop          ( const QString &tableName, IfExists existence = IfExists::ifExists     ) const = 0;
    virtual bool      tableCreate        ( const QString &tableName, IfExists existence = IfExists::ifNotExists  ) const = 0;

                                                                                                            // FIELD,ID
    virtual std::map< QString, int > getDictionaryFromTable( const QString &tableName, const QVector<QString > &fields ) const
    {
        QString queryText = makeSimpleSelectQueryText( tableName, fields );
        bool queryRes = false;
        QSqlQuery executedQuery = execHelper ( queryText, &queryRes );

        if (!queryRes)
            throw std::runtime_error("Something goes wrong in IDatabaseManager::getDictionary (1)");

        QVector< QVector<QString> > vvRes = selectResultToStringVectors( executedQuery );

        std::map< QString, int > resMap;

        for( const auto v : vvRes )
        {
            if (v.size()<2)
               throw std::runtime_error("Something goes wrong in IDatabaseManager::getDictionary (2)");

            bool convertRes = false;
            int val = (int)v[1].toUInt(&convertRes, 10);
            if (!convertRes)
               throw std::runtime_error("Something goes wrong in IDatabaseManager::getDictionary (3)");

            resMap[ v[0].trimmed().toUpper() ] = val;
        }

        return resMap;
    }

    virtual std::map< QString, int > getDictionaryFromTable( const QString &tableName, const QStringList &fields ) const
    {
        return getDictionaryFromTable( tableName, convertToQVectorOfQStrings(fields) );
    }

    virtual std::map< QString, int > getDictionaryFromTable( const QString &tableName, const QString &fields ) const
    {
        return getDictionaryFromTable( tableName, convertToQVectorOfQStrings(fields) );
    }

                                                                                                               // ID,FIELD
    virtual std::map< int, QString > getIdToFieldMapFromTable( const QString &tableName, const QVector<QString > &fields ) const
    {
        QString queryText = makeSimpleSelectQueryText( tableName, fields );
        bool queryRes = false;
        QSqlQuery executedQuery = execHelper ( queryText, &queryRes );

        if (!queryRes)
            throw std::runtime_error("Something goes wrong in IDatabaseManager::getIdToFieldMapFromTable (1)");

        QVector< QVector<QString> > vvRes = selectResultToStringVectors( executedQuery );

        std::map< int, QString > resMap;

        for( const auto v : vvRes )
        {
            if (v.size()<2)
               throw std::runtime_error("Something goes wrong in IDatabaseManager::getIdToFieldMapFromTable (2)");

            bool convertRes = false;
            int key = (int)v[0].toUInt(&convertRes, 10);
            if (!convertRes)
               throw std::runtime_error("Something goes wrong in IDatabaseManager::getIdToFieldMapFromTable (3)");

            resMap[ key ] = v[1].trimmed();
        }

        return resMap;
    }

    virtual std::map< int, QString > getIdToFieldMapFromTable( const QString &tableName, const QStringList &fields ) const
    {
        return getIdToFieldMapFromTable( tableName, convertToQVectorOfQStrings(fields) );
    }

    virtual std::map< int, QString > getIdToFieldMapFromTable( const QString &tableName, const QString &fields ) const
    {
        return getIdToFieldMapFromTable( tableName, convertToQVectorOfQStrings(fields) );
    }



}; // struct IDatabaseManager

//----------------------------------------------------------------------------




} // namespace invest_openapi

