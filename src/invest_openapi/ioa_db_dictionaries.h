/*! \file
    \brief 
 */

//----------------------------------------------------------------------------
#pragma once

#include <QObject>
#include <QDebug>
#include <QtDebug>
#include <QTest>
#include <QTimeZone>
#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlField>
#include <QSqlQuery>
#include <QSharedPointer>

#include <exception>
#include <stdexcept>
#include <sstream>

#include "invest_openapi.h"
#include "qt_time_helpers.h"
#include "marty_decimal.h"
#include "utility.h"

#include "../cpp/cpp.h"

#include "invest_openapi/database_manager.h"



//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
namespace invest_openapi {




//----------------------------------------------------------------------------
class DatabaseDictionaries
{

//------------------------------
public:

    typedef QSharedPointer<IDatabaseManager>  DbManPtr;

    typedef       std::string   stdstr;
    typedef const std::string  cstdstr;

    typedef       QString         qstr;
    typedef const QString        cqstr;

//------------------------------
protected:

    static stdstr sp()                 { return stdstr(" ");      }      // space
    static stdstr cm()                 { return stdstr(",");      }      // comma
    static stdstr fq( cqstr   &qs )    { return qs.toStdString(); }      // from QString helper
    static stdstr fq( cstdstr &qs )    { return qs;               }      // overload for std::string


    //------------------------------
    template< typename CHK > /* CHK - container */
    void notEmptyOrThrow( const CHK &chk, cstdstr &msg )
    {
        if (chk.empty())
            throw std::runtime_error( stdstr("Fucking shit happens, may be DB basic tables are not apropriate filled - ") + msg );
    }

    //------------------------------
    // Я долбаный 0x03-шник, новомодные штучки плохо знаю, поэтому копипастим по-старинке, некогда думать, надо хуярить

    template< typename CHK >
    void notEmptyOrThrow( const CHK &chk, cstdstr &msg1, cstdstr &msg2 )
    {
        notEmptyOrThrow( chk, msg1+sp()+msg2 );
    }

    //------------------------------
    template< typename CHK >
    void notEmptyOrThrow( const CHK &chk, cstdstr &msg1, cstdstr &msg2, cstdstr &msg3 )
    {
        notEmptyOrThrow( chk, msg1, msg2+sp()+msg3 );
    }

    //------------------------------
    template< typename CHK >
    void notEmptyOrThrow( const CHK &chk, cstdstr &msg1, cstdstr &msg2, cstdstr &msg3, cstdstr &msg4 )
    {
        notEmptyOrThrow( chk, msg1, msg2, msg3+sp()+msg4 );
    }

    //------------------------------
    std::map< QString, int >
    loadDictionaryChecked( DbManPtr pDbMan, const QString &table, const QString &nameIdPair )
    {
        std::map< QString, int > dict = pDbMan->getDictionaryFromTable( table, nameIdPair );
        notEmptyOrThrow(dict, stdstr("table:"), fq(table), stdstr(", fileds:"), fq(nameIdPair) );
        //unnormouse
        return dict;
    }

    //------------------------------
    std::map< int, QString >
    loadIdMapChecked( DbManPtr pDbMan, const QString &table, const QString &idNamePair )
    {
        std::map< int, QString > idMap = pDbMan->getIdToFieldMapFromTable( table, idNamePair );
        notEmptyOrThrow(idMap, stdstr("table:"), fq(table), stdstr(", fileds:"), fq(idNamePair) );
        return idMap;
    }

    //------------------------------
    static QString toUpperHelper( const QString &s )  { return s.toUpper(); }
    static int toUpperHelper( int i )                 { return i; }

    //------------------------------
    template <typename MapType >
    bool findInMap( const MapType &m, const typename MapType::key_type &k, typename MapType::mapped_type &id ) const
    {
        //std::map< QString, int >::
        typename MapType::const_iterator it = m.find( toUpperHelper(k) );
        if (it==m.end())
            return false;
        id = it->second;
        return true;
    }

    //------------------------------
protected:

    enum HowToLikeLookup
    {
        lookupUnknown    ,
        lookupExact      ,
        lookupStartsWith ,
        lookupEndsWith   ,
        lookupWith
    };

    static
    HowToLikeLookup prepareLikeLookupCriteria( QString &likeStr )
    {
        if (likeStr.isEmpty())
            return lookupUnknown;

        bool endsNoMean   = false;
        bool startsNoMean = false;

        if (likeStr.endsWith("*") || likeStr.endsWith("%"))
        {
            likeStr.chop(1);
            endsNoMean = true;
        }

        if (likeStr.isEmpty())
            return lookupUnknown;

        if (likeStr.startsWith("*") || likeStr.startsWith("%"))
        {
            likeStr.remove(0,1);
            startsNoMean = true;
        }

        if (likeStr.isEmpty())
            return lookupUnknown;

        if (startsNoMean && endsNoMean)
            return lookupWith;

        else if (!startsNoMean && endsNoMean)
            return lookupStartsWith;

        else if (startsNoMean && !endsNoMean)
            return lookupEndsWith;

        else if (!startsNoMean && !endsNoMean)
            return lookupExact;

        return lookupUnknown;

    }

    //! Compares str with likeStr using HowToLikeLookup htl criteria and case sensitivity cs parameter. Parameter cs is optional and set to Qt::CaseInsensitive by default.
    /*! This function named isStringLike cause classic compare returns int - (-1, 0, 1) as compare result.
        isStringLike, instead, returns true or false, if str matches likeStr or not.

        This function is not ready to be sort predicate.

        \returns true if str==likeStr
     */
    static
    bool isStringLike( const QString &str, const QString &likeStr, HowToLikeLookup htl, Qt::CaseSensitivity cs = Qt::CaseInsensitive )
    {
        switch(htl)
        {
            case lookupExact     : return str.compare(likeStr, cs) == 0;

            case lookupStartsWith: return str.startsWith(likeStr, cs);

            case lookupEndsWith  : return str.endsWith(likeStr, cs);

            case lookupWith      : return str.indexOf(likeStr, cs) >= 0;

            default              : return false;

        };

    }

    template<typename ValueType>
    std::set<QString> findKeyInMapLike( const std::map<QString, ValueType> &m, QString likeStr,  Qt::CaseSensitivity cs = Qt::CaseInsensitive ) const
    {
        std::set<QString> resSet;

        HowToLikeLookup htl = prepareLikeLookupCriteria(likeStr);

        if (htl==lookupUnknown)
            return resSet;


        std::map<QString, ValueType>::const_iterator it = m.begin();
        for(; it!=m.end(); ++it)
        {
            if (isStringLike( it->first, likeStr, htl, cs ))
                resSet.insert(it->first);
        }

        return resSet;
    }

    //------------------------------
    static QString reverseCommaSeparatedString( const QString &s )
    {
        QStringList      lst = s.split( (QChar)',', Qt::KeepEmptyParts );
        QVector<QString> v   = convertToQVectorOfQStrings(lst);
        std::reverse( v.begin(), v.end() );
        return mergeString(v.begin(), v.end(), QString(",") );
    }

    //------------------------------
    static QString normalizeName( const QString &s )
    {
        QString resStr;

        //QString::const_iterator it = s.begin();

        for( auto qch : s )
        {
            char ch = qch.toLatin1();
            switch(ch)
            {
                case '.' : case ',': case '(': case ')': case '&': 
                case '\'': case '`': case '-': case '/': case '!': 
                case '«': case '>':
                    qch = (QChar)' ';
            }

            resStr.append(qch);
        }
        
        return resStr;
    }

    //------------------------------
    static std::set< QString > makeKeysFromName( QString s ) 
    {
        s = normalizeName(s.toUpper());

        QStringList nameParts = s.split( (QChar)' ', Qt::SkipEmptyParts );

        std::set< QString > resSet;
        QString             collectedName;

        for( const auto namePart : nameParts )
        {
            if (!collectedName.isEmpty())
               collectedName.append( (QChar)' ' );

            collectedName.append(namePart);

            resSet.insert(collectedName);

            if (namePart.size()>1)
                resSet.insert(namePart);

        }

        return resSet;

    }

    //------------------------------
    void loadFigiDictionaries( DbManPtr pDbMan )
    {

        figiToId      = loadDictionaryChecked( pDbMan, "MARKET_INSTRUMENT", "FIGI,ID"   );
        tickerToId    = loadDictionaryChecked( pDbMan, "MARKET_INSTRUMENT", "TICKER,ID" );
        isinToId      = loadDictionaryChecked( pDbMan, "MARKET_INSTRUMENT", "ISIN,ID"   );

        auto figiToCurrencyId = loadDictionaryChecked( pDbMan, "MARKET_INSTRUMENT", "FIGI,CURRENCY_ID" );

        

        //std::map< int, QString > 
        idToFigi      = makeMapSwapKeyVal( figiToId );
        tickerToFigi  = makeTransitionMap( tickerToId, idToFigi   );
        isinToFigi    = makeTransitionMap( isinToId  , idToFigi   );

        std::map< int, QString > idToTicker    = makeMapSwapKeyVal( tickerToId );
        figiToTicker                           = makeTransitionMap( figiToId  , idToTicker );

        std::map< int, QString > idToFigiName  = loadIdMapChecked( pDbMan, "MARKET_INSTRUMENT", "ID,NAME"   );
        figiToName                             = makeTransitionMap( figiToId  , idToFigiName );

        for( auto figiCurrency : figiToCurrencyId )
        {
            const QString &figi = figiCurrency.first;
            int currencyId      = figiCurrency.second;

            auto foundIt = figiToId.find(figi);
            if (foundIt == figiToId.end())
                continue;

            idToCurrencyId[foundIt->second] = currencyId;
        }

        //std::map< QString, std::set<QString> > nameToFigiSet;

        nameToFigiSet.clear();

        std::map< int, QString >::const_iterator namesIt = idToFigiName.begin();
        for(; namesIt != idToFigiName.end(); ++namesIt)
        {
            int id = namesIt->first;

            QString idFigi;

            if (!findInMap( idToFigi, id, idFigi ))
                continue;

            std::set< QString > keySet = makeKeysFromName( normalizeName(namesIt->second) );

            for( auto key : keySet )
            {
                key = toUpperHelper(key);
                nameToFigiSet[key].insert(idFigi);
            }
        }

    }


//------------------------------
public:

    //------------------------------
    void dictLoadTest( DbManPtr pDbMan, const QString &table, const QString &nameIdPair )
    {
        loadDictionaryChecked( pDbMan, table, nameIdPair );
    }

    //------------------------------
    QString findFigiByAnyIdString( QString idStr ) const
    {
        std::string idStdStr = idStr.toStdString();

        idStr = QString::fromStdString(idStdStr);

        idStr = idStr.toUpper();

        QString figi;

        if (findInMap( tickerToFigi, idStr, figi ))
            return figi;

        int figiKey = 0;
        if (findInMap( figiToId, idStr, figiKey ))
            return idStr; // FIGI taken itself

        if (findInMap( isinToFigi, idStr, figi ))
            return figi;



        if (idStr.indexOf("*")<0 && idStr.indexOf("%")<0)
            idStr += "*";

        std::set<QString> likes = findKeyInMapLike( tickerToFigi, idStr );
        if (likes.size()==1)
        {
            if (findInMap( tickerToFigi, *(likes.begin()), figi ))
                return figi;
        }

        likes = findKeyInMapLike( figiToId, idStr );
        if (likes.size()==1)
            return *(likes.begin());

        likes = findKeyInMapLike( isinToFigi, idStr );
        if (likes.size()==1)
        {
            if (findInMap( isinToFigi, *(likes.begin()), figi ))
                return figi;
        }


        // Not any valid ID
        return QString();
    }

    //------------------------------
    // Throws an exception if FIGI not found
    QString findFigiByAnyIdStringChecked( QString idStr ) const
    {
        QString figi = findFigiByAnyIdString( idStr );
        if (figi.isEmpty())
            throw std::runtime_error("findFigiByAnyIdStringChecked: FIGI not found");
        return figi;
    }

    //------------------------------
    QString getTickerByFigi( QString figi ) const
    {
        QString ticker;

        if (findInMap( figiToTicker, figi.toUpper(), ticker ))
            return ticker;

        return ticker;
    }

    QString getTickerByFigiChecked( QString figi ) const
    {
        QString ticker = getTickerByFigi(figi);

        if (ticker.isEmpty())
            throw std::runtime_error("getTickerByFigiChecked: FIGI not found");

        return ticker;
    }

    //------------------------------
    // Throws an exception if FIGI not found
    QString getNameByFigiChecked( QString figi ) const
    {
        QString name;

        if (findInMap( figiToName, figi.toUpper(), name ))
            return name;

        throw std::runtime_error("getTickerByFigi: FIGI not found");

        return QString();
    }

    //------------------------------
    std::set<QString> findFigisByName( QString name ) const
    {
        name = name.toUpper();

        std::set<QString> figiSet;

        // Lookup for exact value
        if (findInMap( nameToFigiSet, name, figiSet ))
            return figiSet;

        // Exact value not found
        std::map< QString, std::set<QString> >::const_iterator it = nameToFigiSet.begin();

        for(; it != nameToFigiSet.end(); ++it)
        {
            int idx = it->first.indexOf( name,  /* from = */  0, Qt::CaseInsensitive );

            if (idx<0)
                continue;

            figiSet.insert( it->second.begin(), it->second.end() );
        }

        return figiSet;
    }

    int getInstrumentCurrencyId( int instrumentId ) const
    {
        std::map< int, int >::const_iterator it = idToCurrencyId.find(instrumentId);
        if (it==idToCurrencyId.end())
            return -1;
        return it->second;
    }

    int getInstrumentCurrencyId( const QString &figi ) const
    {
        int instrumentId = getInstrumentId(figi);
        if (instrumentId<0)
            return -1;
        return getInstrumentCurrencyId(instrumentId);
    }

    //std::map< int, QString > idToCurrencyId   ;

    /*
    QString getInstrumentFigiById( int id ) const
    {
        QString res;
        if (findInMap( idToFigi, id, res ))
            return res;
        return QString();
    }
    */

    /*
    int getInstrumentIdBegin( ) const
    {
        if (idToStrMap.empty())
            return 0;
        return idToStrMap.begin()->first;
    }

    int getInstrumentIdEnd( ) const
    {
        if (idToStrMap.empty())
            return 0;
        return idToStrMap.rbegin()->first + 1;
    }
    */


    #define IOA_DB_DICTIONARIES_DEFINE_GET_ID_GET_BY_ID_METHODS( valueType, strToIdMap, idToStrMap )                   \
                                                                                                                       \
                    int get ##valueType ##IdBegin( ) const                                                             \
                    {                                                                                                  \
                        if (idToStrMap.empty())                                                                        \
                            return 0;                                                                                  \
                        return idToStrMap.begin()->first;                                                              \
                    }                                                                                                  \
                                                                                                                       \
                    int get ##valueType ##IdEnd( ) const                                                               \
                    {                                                                                                  \
                        if (idToStrMap.empty())                                                                        \
                            return 0;                                                                                  \
                        return idToStrMap.rbegin()->first + 1;                                                         \
                    }                                                                                                  \
                                                                                                                       \
                    int get ##valueType ##Id( const QString &str ) const                                               \
                    {                                                                                                  \
                        int id = -1;                                                                                   \
                        if (findInMap( strToIdMap, str.toUpper(), id ))                                                \
                            return id;                                                                                 \
                        return -1;                                                                                     \
                    }                                                                                                  \
                                                                                                                       \
                    int get ##valueType ##IdChecked( const QString &str ) const                                        \
                    {                                                                                                  \
                        int id = get ##valueType ##Id(str);                                                            \
                        if (id<0)                                                                                      \
                            throw std::runtime_error("get" #valueType "IdChecked: " #valueType " not found");          \
                        return id;                                                                                     \
                    }                                                                                                  \
                                                                                                                       \
                    QString get ##valueType ##ById( int id ) const                                                     \
                    {                                                                                                  \
                        QString res;                                                                                   \
                        if (findInMap( idToStrMap, id, res ))                                                          \
                            return res;                                                                                \
                        return QString();                                                                              \
                    }                                                                                                  \
                                                                                                                       \
                    QString get ##valueType ##ByIdChecked( int id ) const                                              \
                    {                                                                                                  \
                        QString res = get ##valueType ##ById( id );                                                    \
                        if (res.isEmpty())                                                                             \
                            throw std::runtime_error("get" #valueType "ByIdChecked: " #valueType " dy Id not found");  \
                        return res;                                                                                    \
                    }


    IOA_DB_DICTIONARIES_DEFINE_GET_ID_GET_BY_ID_METHODS( Instrument, figiToId, idToFigi )


    #define IOA_DB_DICTIONARIES_DEFINE_LOAD_DICTIONARY( valueType, strToIdMap, idToStrMap, tableName, fields )        \
                                                                                                                      \
                protected:                                                                                            \
                    std::map< QString, int > strToIdMap   ;                                                           \
                    std::map< int, QString > idToStrMap   ;                                                           \
                                                                                                                      \
                    void load ##valueType ##Dictionary( DbManPtr pDbMan )                                             \
                    {                                                                                                 \
                        strToIdMap = loadDictionaryChecked( pDbMan, tableName, reverseCommaSeparatedString(fields) ); \
                        /* idToStrMap = makeMapSwapKeyVal( strToIdMap ); */                                           \
                        idToStrMap = loadIdMapChecked( pDbMan, tableName, fields );                                   \
                    }


    #define IOA_DB_DICTIONARIES_DEFINE_LOAD_AND_GETTERS( valueType, strToIdMap, idToStrMap, tableName, fields )         \
                protected:                                                                                              \
                    IOA_DB_DICTIONARIES_DEFINE_LOAD_DICTIONARY( valueType, strToIdMap, idToStrMap, tableName, fields )  \
                public:                                                                                                 \
                    IOA_DB_DICTIONARIES_DEFINE_GET_ID_GET_BY_ID_METHODS( valueType, strToIdMap, idToStrMap )



    IOA_DB_DICTIONARIES_DEFINE_LOAD_AND_GETTERS( BrokerAccountType           , brokerAccountTypeToId           , idToBrokerAccountType           , "BROKER_ACCOUNT_TYPE"           , "ID,TYPE" )

    IOA_DB_DICTIONARIES_DEFINE_LOAD_AND_GETTERS( CandleResolution            , candleResolutionToId            , idToCandleResolution            , "CANDLE_RESOLUTION"             , "ID,RESOLUTION" )
    // Тут нормально работает только get*ById, потому что строки не уникальны
    // Но другое и не нужно, ID берём от CandleResolution
    IOA_DB_DICTIONARIES_DEFINE_LOAD_AND_GETTERS( CandleResolutionIntervalMin , candleResolutionIntervalMinToId , idToCandleResolutionIntervalMin , "CANDLE_RESOLUTION"             , "ID,INTERVAL_MIN" )
    IOA_DB_DICTIONARIES_DEFINE_LOAD_AND_GETTERS( CandleResolutionIntervalMax , candleResolutionIntervalMaxToId , idToCandleResolutionIntervalMax , "CANDLE_RESOLUTION"             , "ID,INTERVAL_MAX" )
    IOA_DB_DICTIONARIES_DEFINE_LOAD_AND_GETTERS( CandleResolutionIntervalRec , candleResolutionIntervalRecToId , idToCandleResolutionIntervalRec , "CANDLE_RESOLUTION"             , "ID,INTERVAL_RECOMENDED_INCREMENT" )

    IOA_DB_DICTIONARIES_DEFINE_LOAD_AND_GETTERS( Currency                    , currencyToId                    , idToCurrency                    , "CURRENCY"                      , "ID,NAME" )

    IOA_DB_DICTIONARIES_DEFINE_LOAD_AND_GETTERS( InstrumentType              , instrumentTypeToId              , idToInstrumentType              , "INSTRUMENT_TYPE"               , "ID,TYPE" )

    IOA_DB_DICTIONARIES_DEFINE_LOAD_AND_GETTERS( OperationStatus             , operationStatusToId             , idToOperationStatus             , "OPERATION_STATUS"              , "ID,STATUS" )

    IOA_DB_DICTIONARIES_DEFINE_LOAD_AND_GETTERS( OperationType               , operationTypeToId               , idToOperationType               , "OPERATION_TYPE"                , "ID,TYPE" )

    IOA_DB_DICTIONARIES_DEFINE_LOAD_AND_GETTERS( OperationTypeWithCommission , operationTypeWithCommissionToId , idToOperationTypeWithCommission , "OPERATION_TYPE_WITH_COMMISSION", "ID,TYPE" )

    IOA_DB_DICTIONARIES_DEFINE_LOAD_AND_GETTERS( OrderStatus                 , orderStatusToId                 , idToOrderStatus                 , "ORDER_STATUS", "ID,STATUS" )

    IOA_DB_DICTIONARIES_DEFINE_LOAD_AND_GETTERS( OrderType                   , orderTypeToId                   , idToOrderType                   , "ORDER_TYPE", "ID,TYPE" )

    IOA_DB_DICTIONARIES_DEFINE_LOAD_AND_GETTERS( StockExangeList             , stockExangeListToId             , idToStockExangeList             , "STOCK_EXCHANGE_LIST", "ID,NAME" )



    bool isValidId( const QString & s ) const
    {
        if (s.isEmpty())
            return false;

        if (s.toUpper()=="INVALID")
            return false;

        return true;
    }

    bool isValidId( int id ) const
    {
        if (id==-1)
            return false;
        return true;
    }

    /*
    template< typename TypeName >
    bool isValidId( int id, const TypeName &t ) const
    {
        if (id==0)
            return false;
        if (id==-1)
            return false;
        return true;
    }

    template< >
    bool isValidId<MarketInstrument>( int id, const MarketInstrument &t ) const
    {
        if (id==-1)
            return false;
        return true;
    }
    */


//------------------------------
protected:

    //------------------------------
    // FIGI

    std::map< QString, int > figiToId         ;
    std::map< QString, int > tickerToId       ;
    std::map< QString, int > isinToId         ;

    std::map< int, QString > idToFigi         ;
    std::map< int, int     > idToCurrencyId   ;

    //std::map< int, QString > idToFigiName     ;

    std::map< QString, QString > tickerToFigi ;
    std::map< QString, QString > isinToFigi   ;
    std::map< QString, QString > figiToTicker ;
    std::map< QString, QString > figiToName   ;

    std::map< QString, std::set<QString> > nameToFigiSet;


//------------------------------
public:


    //------------------------------
    // Called from CTOR, use it for reload/update if DB dictionaries was changed
    void loadAllDictionaries( DbManPtr pDbMan )
    {
        loadFigiDictionaries( pDbMan );

        loadBrokerAccountTypeDictionary( pDbMan );

        loadCandleResolutionDictionary( pDbMan );
        loadCandleResolutionIntervalMinDictionary( pDbMan );
        loadCandleResolutionIntervalMaxDictionary( pDbMan );
        loadCandleResolutionIntervalRecDictionary( pDbMan );

        loadCurrencyDictionary( pDbMan );

        loadInstrumentTypeDictionary( pDbMan );

        loadOperationStatusDictionary( pDbMan );

        loadOperationTypeDictionary( pDbMan );

        loadOperationTypeWithCommissionDictionary( pDbMan );

        loadOrderStatusDictionary( pDbMan );

        loadOrderTypeDictionary( pDbMan );

        loadStockExangeListDictionary( pDbMan );

        //load ##valueType ##Dictionary( pDbMan );

        //load ##valueType ##Dictionary( pDbMan );

        //load ##valueType ##Dictionary( pDbMan );

        //load ##valueType ##Dictionary( pDbMan );

        //load ##valueType ##Dictionary( pDbMan );

        //load ##valueType ##Dictionary( pDbMan );
    }

    //------------------------------
    DatabaseDictionaries() {}

    //------------------------------
    DatabaseDictionaries( DbManPtr pDbMan )
    {
        loadAllDictionaries( pDbMan );
    }



}; // class DatabaseDictionaries

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------

} // namespace invest_openapi


