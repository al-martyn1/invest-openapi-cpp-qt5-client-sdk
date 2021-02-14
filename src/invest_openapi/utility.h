/*! \file
    \brief 
 */

#pragma once

#include <QString>
#include <QStringList>
#include <QVector>
#include <QDebug>
#include <QNetworkReply>

#if defined(QT_SQL_LIB)
    #include <QSqlDatabase>
    #include <QSqlDriver>
    #include <QSqlField>
    #include <QSqlError>
#endif

#include <exception>
#include <stdexcept>
#include <map>
#include <set>

#include <algorithm>
#include <iterator>


#include "models.h"

#include "openapi_completable_future_base.h"
#include "openapi_completable_future.h"

#include "marty_decimal.h"


namespace invest_openapi
{

void pollMessageQueue();

/*
inline
QVector<QString> toVector( const QStringList &ql )
{
    QVector<QString> res;

    for (int i = 0; i < ql.size(); ++i)
    {
        QString s = ql.at(i);
        res.append(s);
    }

    return res;
}
*/



//----------------------------------------------------------------------------
template <typename StringType> inline bool starts_with( const StringType &str, const StringType &prefix )
{
    if (str.size()<prefix.size())
        return false;

    return str.compare( 0, prefix.size(), prefix )==0;
}

//------------------------------
template <typename StringType> inline bool ends_with( const StringType &str, const StringType &postfix )
{
    if (str.size()<postfix.size())
        return false;

    return str.compare( str.size()-postfix.size(), postfix.size(), postfix )==0;
}

//----------------------------------------------------------------------------
inline QString makeQStringFromStdString( const std::string &str, std::string::size_type pos = 0, std::string::size_type cnt = std::string::npos )
{
    return QString::fromStdString( std::string( str, pos, cnt ) );
}

inline QString makeQStringFromStdString( const std::wstring &str, std::wstring::size_type pos = 0, std::wstring::size_type cnt = std::wstring::npos )
{
    return QString::fromStdWString( std::wstring( str, pos, cnt ) );
}

//----------------------------------------------------------------------------


template <typename StringType> inline 
QVector<QString> splitByCharWithBraces( const StringType &str, typename StringType::value_type sepCh )
{
    typedef typename StringType::value_type CharType;
    typedef typename StringType::size_type  SizeType;

    QVector<QString> resVec;

    int bracesLevel = 0;

    SizeType segmentStartPos = 0;

    for( SizeType i=0; i!=str.size(); ++i)
    {
        if (str[i]==(CharType)'(')
        {
            ++bracesLevel;
        }
        else if (str[i]==(CharType)')')
        {
            --bracesLevel;
        }
        else if (str[i]==sepCh)
        {
            if (!bracesLevel)
            {
                // found separator
                auto len = i - segmentStartPos;
                resVec.push_back( makeQStringFromStdString( str, segmentStartPos, len ) );
                segmentStartPos = i+1;
            }
        }
        else
        {
            // do nothing on regular symbol
        }
    } // for

    resVec.push_back( makeQStringFromStdString( str, segmentStartPos ) );

    return resVec;
}


//----------------------------------------------------------------------------
template< typename T >
inline
bool getOpenApiCompletableFuturesCompletedAll( const QVector< QSharedPointer< OpenApiCompletableFuture< T > > > &futures )
{
    for( const auto &f : futures )
    {
        if (!f->isCompleted())
            return false;
    }

    return true;
}

//----------------------------------------------------------------------------
template< typename T >
inline
bool getOpenApiCompletableFuturesCompletedAny( const QVector< QSharedPointer< OpenApiCompletableFuture< T > > > &futures )
{
    for( const auto &f : futures )
    {
        if (f->isCompleted())
            return trtue;
    }

    return false;
}

//----------------------------------------------------------------------------
template< typename T >
inline
void joinOpenApiCompletableFutures( const QVector< QSharedPointer< OpenApiCompletableFuture< T > > > &futures )
{
    while(!getOpenApiCompletableFuturesCompletedAll(futures)) { pollMessageQueue(); }
}

//----------------------------------------------------------------------------
template<typename T>
inline
void checkAbort( const QSharedPointer< OpenApiCompletableFuture< T > > &val )
{
    if (!val->isCompletionError())
        return;

    dumpIfError(val);

    throw std::runtime_error("Something goes wrong");
}

//----------------------------------------------------------------------------
inline QString listStringNormalize( QString s )
{
    s.replace(':', ";"); // replace *nix style list separator to windows style separator
    s.replace(',', ";"); // replace commas to ';' (windows style separator)
    return s;
}

//------------------------------
inline QStringList listStringNormalize( const QStringList s )
{
    QStringList resList;
    for( auto str : s ) resList.push_back(listStringNormalize(str));
    return resList;
}

//------------------------------
inline QVector<QStringList> listStringNormalize( const QVector<QStringList> &s )
{
    QVector<QStringList> resList;
    for( auto str : s ) resList.push_back(listStringNormalize(str));
    return resList;
}

//----------------------------------------------------------------------------
inline QStringList listStringTrim( const QStringList &l )
{
    QStringList resList;
    for( auto s : l )
    {
        resList.push_back(s.trimmed());
    }

    return resList;
}

//----------------------------------------------------------------------------
inline QStringList listStringSplit( QString s )
{
    s = listStringNormalize(s);
    return listStringTrim( s.split( ';', Qt::KeepEmptyParts ) );
}

//------------------------------
inline QVector<QStringList> listStringSplit( const QStringList &s )
{
    QVector<QStringList> resList;
    for( auto str : s ) resList.push_back(listStringSplit(str));
    return resList;
}

//------------------------------
inline QVector<QStringList> listStringSplit( const QVector<QString> &s )
{
    QVector<QStringList> resList;
    for( auto str : s ) resList.push_back(listStringSplit(str));
    return resList;
}

//----------------------------------------------------------------------------
inline QStringList pathListSplit( QString pathList )
{
    pathList.replace(':', ";"); // replace *nix style list separator to windows style separator
    return pathList.split( ';', Qt::SkipEmptyParts );
}

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
inline QVector<QString> convertToQVectorOfQStrings( const QVector<QString> &stringList )
{
    return stringList;
}

//------------------------------
inline QVector<QString> convertToQVectorOfQStrings( const QVector<QVariant> &variantList )
{
    QVector<QString> strVec;
    for( auto v : variantList ) strVec.push_back(v.toString());
    return strVec;
}

//------------------------------
inline QVector<QString> convertToQVectorOfQStrings( const QStringList strList )
{
    QVector<QString> strVec;
    for( auto str : strList ) strVec.push_back(str);
    return strVec;
}

//------------------------------
inline QVector<QString> convertToQVectorOfQStrings( const QString &listStr )
{
    QString     normalizedListStr = listStringNormalize(listStr);
    QStringList strList = listStringSplit(normalizedListStr);
    return convertToQVectorOfQStrings(strList);
}

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
inline QVector< QVector<QString> > convertToQVectorOfQVectorOfQStrings( const QVector< QVector<QString> > &lst )
{
    return lst;
}

//------------------------------
inline QVector< QVector<QString> > convertToQVectorOfQVectorOfQStrings( const QVector< QVector<QVariant> > &lst )
{
    QVector< QVector<QString> > resVec;
    for( auto v : lst ) resVec.push_back(convertToQVectorOfQStrings(v));
    return resVec;
}

//------------------------------
inline QVector< QVector<QString> > convertToQVectorOfQVectorOfQStrings( const QVector< QStringList > &lst )
{
    QVector< QVector<QString> > resVec;
    for( auto sl : lst ) resVec.push_back(convertToQVectorOfQStrings(sl));
    return resVec;
}

//------------------------------
inline QVector< QVector<QString> > convertToQVectorOfQVectorOfQStrings( const QVector< QString > &lst )
{
    QVector< QVector<QString> > resVec;
    //for( auto s : lst ) resVec.push_back(convertToQVectorOfQStrings(s));
    resVec.push_back(lst);
    return resVec;
}

//------------------------------
inline QVector< QVector<QString> > convertToQVectorOfQVectorOfQStrings( const QVector< QString > &lst, const QString &splitBy )
{
    QVector< QVector<QString> > resVec;
    for( auto s : lst ) resVec.push_back(convertToQVectorOfQStrings( s.split(splitBy) ));
    return resVec;
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
#define INVEST_OPENAPI_TO_TYPE_CONVERTER_FROM_QSTRING( TypeTo ) \
            TypeTo res;                                         \
            res.fromJson(v);                                    \
            return res

#define INVEST_OPENAPI_TO_TYPE_CONVERTER_FROM_ENUM( TypeTo )    \
            TypeTo res;                                         \
            res.setValue(v);                                    \
            return res;

#define INVEST_OPENAPI_TO_TYPE_CONVERTER_FROM_UNDEFINED( methodName, UndefinedTypeName ) \
            throw std::runtime_error("template<> invest_openapi::" #methodName "( " #UndefinedTypeName " ) not implemented for this type")

//----------------------------------------------------------------------------
/*
template< typename CurrencySourceType > 
            inline Currency             toCurrency                             (const CurrencySourceType           &v) { INVEST_OPENAPI_TO_TYPE_CONVERTER_FROM_UNDEFINED(toCurrency,CurrencySourceType); }
template< > inline Currency             toCurrency<Currency                   >(const Currency                     &v) { return v; }
template< > inline Currency             toCurrency<Currency::eCurrency        >(const Currency::eCurrency          &v) { INVEST_OPENAPI_TO_TYPE_CONVERTER_FROM_ENUM( Currency ); }
template< > inline Currency             toCurrency<QString                    >(const QString                      &v) { INVEST_OPENAPI_TO_TYPE_CONVERTER_FROM_STRING( Currency ); }
*/

#define INVEST_OPENAPI_TO_TYPE_CONVERTER( methodName, targetType, targetTypeEnum, templateTypeName )                                                                                                                         \
               template< typename templateTypeName >                                                                                                                                                                         \
                           inline targetType             methodName                              (const templateTypeName              &v) { INVEST_OPENAPI_TO_TYPE_CONVERTER_FROM_UNDEFINED(methodName,templateTypeName); }  \
               template< > inline targetType             methodName<targetType                  >(const targetType                    &v) { return v; }                                                                      \
               template< > inline targetType             methodName<targetType::targetTypeEnum  >(const targetType :: targetTypeEnum  &v) { INVEST_OPENAPI_TO_TYPE_CONVERTER_FROM_ENUM( targetType ); }                      \
               template< > inline targetType             methodName<QString                     >(const QString                       &v) { INVEST_OPENAPI_TO_TYPE_CONVERTER_FROM_QSTRING( targetType ); }                   \
               template< > inline targetType             methodName<std::string                 >(const std::string                   &v) { return methodName( QString::fromStdString(v) ); }                                \
               template< > inline targetType             methodName<int                         >(const int                           &v) { return methodName( (targetType :: targetTypeEnum)v ); }                          \
                           inline targetType             methodName                              (const char*                          v) { return methodName( QString::fromLatin1(v) ); }


#define INVEST_OPENAPI_TO_INT_CONVERTER( sourceType, sourceEnumType )                             \
               inline int toInt( sourceType :: sourceEnumType f ) { return (int)f; }              \
               inline int toInt( const sourceType &f )            { return toInt(f.getValue()); }


#define INVEST_OPENAPI_TO_QSTRING_CONVERTER( sourceType, sourceEnumType )                                                       \
               inline QString toQString( const sourceType &f )            { return f.asJson().toUpper(); }                      \
               inline QString toQString( sourceType :: sourceEnumType f ) { sourceType s; s.setValue(f); return toQString(s); }


#define INVEST_OPENAPI_TO_STD_STRING_CONVERTER( sourceType, sourceEnumType )                                                    \
               inline std::string toStdString( const sourceType &f )            { return toQString(f).toStdString(); }          \
               inline std::string toStdString( sourceType :: sourceEnumType f ) { return toQString(f).toStdString(); }


#define INVEST_OPENAPI_TO_STRINGS_CONVERTER( sourceType, sourceEnumType )           \
               INVEST_OPENAPI_TO_QSTRING_CONVERTER( sourceType, sourceEnumType )    \
               INVEST_OPENAPI_TO_STD_STRING_CONVERTER( sourceType, sourceEnumType )

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
inline int toInt( int f ) { return f; }
template<typename TInt> inline QString       toQString  ( TInt i ) { return QString::number(i); }
template<typename TInt> inline std::string   toStdString( TInt i ) { return QString::number(i).toStdString(); }

INVEST_OPENAPI_TO_TYPE_CONVERTER( toCurrency           , Currency           , eCurrency          , CurrencySourceType   )
INVEST_OPENAPI_TO_TYPE_CONVERTER( toInstrumentType     , InstrumentType     , eInstrumentType    , InstrumentSourceType )
INVEST_OPENAPI_TO_TYPE_CONVERTER( toCandleResolution   , CandleResolution   , eCandleResolution  , CandleResolutionSourceType )
INVEST_OPENAPI_TO_TYPE_CONVERTER( toOperationType      , OperationType      , eOperationType     , OperationSourceType )

INVEST_OPENAPI_TO_INT_CONVERTER( Currency           , eCurrency         )
INVEST_OPENAPI_TO_INT_CONVERTER( InstrumentType     , eInstrumentType   )
INVEST_OPENAPI_TO_INT_CONVERTER( CandleResolution   , eCandleResolution )
INVEST_OPENAPI_TO_INT_CONVERTER( OperationType      , eOperationType    )

INVEST_OPENAPI_TO_STRINGS_CONVERTER( Currency           , eCurrency         )
INVEST_OPENAPI_TO_STRINGS_CONVERTER( InstrumentType     , eInstrumentType   )
INVEST_OPENAPI_TO_STRINGS_CONVERTER( CandleResolution   , eCandleResolution )
INVEST_OPENAPI_TO_STRINGS_CONVERTER( OperationType      , eOperationType    )

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
template<typename MonetaryType>
struct Instrument;

//----------------------------------------------------------------------------
template<typename MonetaryType>
inline
Instrument<MonetaryType> toInstrument( const Instrument<MonetaryType> &i )
{
    return i;
}

//----------------------------------------------------------------------------
template<typename MonetaryType>
inline
Instrument<MonetaryType> toInstrument( const MarketInstrument &i )
{
    return Instrument<MonetaryType>(i);
}

//----------------------------------------------------------------------------
template<typename MonetaryType>
inline
MarketInstrument toMarketInstrument( const Instrument<MonetaryType> &i )
{
    return MarketInstrument(i);
}

//----------------------------------------------------------------------------
inline
MarketInstrument toMarketInstrument( const MarketInstrument &i )
{
    return i;
}

//----------------------------------------------------------------------------






//----------------------------------------------------------------------------
template<typename MonetaryType>
inline
QList< Instrument<MonetaryType> > toInstrumentList( const QList<MarketInstrument> &l )
{
    QList< Instrument<MonetaryType> > res;
    for( const auto &li : l )
    {
        res.push_back( Instrument<MonetaryType>(li) );
    }

    return res;
}

//----------------------------------------------------------------------------
template<typename MonetaryType>
inline
QList<MarketInstrument> toMarketInstrumentList( const QList< Instrument<MonetaryType> > &l )
{
    QList<MarketInstrument> res;
    for( const auto &li : l )
    {
        res.push_back(li);
    }

    return res;
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
//#define IOA_DEFINE_INSTRUMENT_ATTRS_MAP_MAKE_FUNCTION( instrumentType, methodName, fieldMapFrom, fieldMapTo )





//----------------------------------------------------------------------------
template<typename MonetaryType>
inline
std::map<QString,QString> makeTickerFigiMap( const QList< Instrument<MonetaryType> > &list )
{
    std::map<QString,QString> resMap;

    for(const auto &instrument : list)
    {
        auto key = instrument.ticker.toUpper();
        if (key.isEmpty())
            continue;
        resMap[key] = instrument.figi;
    }

    return resMap;
}

//----------------------------------------------------------------------------
inline
std::map<QString,QString> makeTickerFigiMap( const QList<MarketInstrument> &list )
{
    std::map<QString,QString> resMap;

    for(const auto &instrument : list)
    {
        auto key = instrument.getTicker().toUpper();
        if (key.isEmpty())
            continue;
        resMap[key] = instrument.getFigi();
    }

    return resMap;
}

//----------------------------------------------------------------------------
template<typename MonetaryType>
inline
std::map<QString,QString> makeTickerIsinMap( const QList< Instrument<MonetaryType> > &list )
{
    std::map<QString,QString> resMap;

    for(const auto &instrument : list)
    {
        auto key = instrument.ticker.toUpper();
        if (key.isEmpty())
            continue;
        resMap[key] = instrument.isin;
    }

    return resMap;
}

//----------------------------------------------------------------------------
template<typename MonetaryType>
inline
std::map<QString,QString> makeFigiTickerMap( const QList< Instrument<MonetaryType> > &list )
{
    std::map<QString,QString> resMap;

    for(const auto &instrument : list)
    {
        auto key = instrument.figi.toUpper();
        if (key.isEmpty())
            continue;
        resMap[key] = instrument.ticker;
    }

    return resMap;
}

//----------------------------------------------------------------------------
inline
std::map<QString,QString> makeFigiTickerMap( const QList<MarketInstrument> &list )
{
    std::map<QString,QString> resMap;

    for(const auto &instrument : list)
    {
        auto key = instrument.getFigi().toUpper();
        if (key.isEmpty())
            continue;
        resMap[key] = instrument.getTicker();
    }

    return resMap;
}

//----------------------------------------------------------------------------
template<typename MonetaryType>
inline
std::map<QString,QString> makeFigiIsinMap( const QList< Instrument<MonetaryType> > &list )
{
    std::map<QString,QString> resMap;

    for(const auto &instrument : list)
    {
        auto key = instrument.figi.toUpper();
        if (key.isEmpty())
            continue;
        resMap[key] = instrument.isin;
    }

    return resMap;
}

//----------------------------------------------------------------------------
template<typename MonetaryType>
inline
std::map<QString,QString> makeIsinTickerMap( const QList< Instrument<MonetaryType> > &list )
{
    std::map<QString,QString> resMap;

    for(const auto &instrument : list)
    {
        auto key = instrument.isin.toUpper();
        if (key.isEmpty())
            continue;
        resMap[key] = instrument.ticker;
    }

    return resMap;
}

//----------------------------------------------------------------------------
inline
std::map<QString,QString> makeIsinTickerMap( const QList<MarketInstrument> &list )
{
    std::map<QString,QString> resMap;

    for(const auto &instrument : list)
    {
        // getIsin()
        // getFigi()
        // getTicker()
        auto key = instrument.getIsin().toUpper();
        if (key.isEmpty())
            continue;
        resMap[key] = instrument.getTicker();
    }

    return resMap;
}

//----------------------------------------------------------------------------
template<typename MonetaryType>
inline
std::map<QString,QString> makeIsinFigiMap( const QList< Instrument<MonetaryType> > &list )
{
    std::map<QString,QString> resMap;

    for(const auto &instrument : list)
    {
        auto key = instrument.isin.toUpper();
        if (key.isEmpty())
            continue;
        resMap[key] = instrument.figi;
    }

    return resMap;
}

//----------------------------------------------------------------------------
inline
std::map<QString,QString> makeIsinFigiMap( const QList<MarketInstrument> &list )
{
    std::map<QString,QString> resMap;

    for(const auto &instrument : list)
    {
        // getIsin()
        // getFigi()
        // getTicker()
        auto key = instrument.getIsin().toUpper();
        if (key.isEmpty())
            continue;
        resMap[key] = instrument.getFigi();
    }

    return resMap;
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
#if 0
//! Базовый false-тип для детекта наличия метода getPayload() у объекта
template< typename C, typename = void >
struct has_getPayload
  : std::false_type
{};

//------------------------------
//! Специализация, тестирующая наличие метода getPayload() у объекта
template< typename C >
struct has_getPayload< C, std::enable_if_t<
                         std::is_same<
                           decltype( std::declval<C>().getPayload( ) ),
                           void
                         >::value
                       > >
  : std::true_type
{};

//------------------------------
//! Функция возвращает payload после join'а
/*! 
     \tparam C Тип контейнера
 */
template< typename C >
std::enable_if_t< has_getPayload< C >::value > inline
getPayload( C& c )
{
  c.reserve( c.size() + n );
}
#endif

// https://en.cppreference.com/w/cpp/language/auto
template< typename ResponseType >
inline
auto joinAndGetPayload( ResponseType response ) -> decltype(response->value.getPayload())
{
    response->join();
    checkAbort(response);
    return response->value.getPayload();
}

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
inline QVector<QString> makeVector(const QStringList      &v, bool bUpper = true )
{
    QVector<QString> resVec;
    for (QStringList::const_iterator it = v.constBegin(); it != v.constEnd(); ++it)
    {
        QString curStr = (bUpper ? it->toUpper() : *it);
        resVec.push_back(curStr);
    }

    return resVec;
}

//----------------------------------------------------------------------------
inline QVector<QString> toStringVector(const QStringList      &v )
{
    return makeVector(v, false);
}

//----------------------------------------------------------------------------
inline QVector<QString> toStringVector(const QVector<QVariant> &v )
{
    QVector<QString> resVec;

    std::transform( v.begin(), v.end(), std::back_inserter(resVec)
                  , []( const QVariant &q ) -> QString
                    {
                        if (q.isNull() || !q.isValid())
                            return QString();
                        return q.toString();
                    }
                  );

    return resVec;
}

//----------------------------------------------------------------------------
inline QVector<QVector<QString> > toStringVector(const QVector<QVector<QVariant> > &v )
{
    QVector<QVector<QString> > resVec;

    std::transform( v.begin(), v.end(), std::back_inserter(resVec)
                  , []( const QVector<QVariant> &v)
                    {
                        return toStringVector(v);
                    }
                  );

    return resVec;
}

//----------------------------------------------------------------------------
inline QVector<QString> toStringVector(const QList<QString> &v )
{
    QVector<QString> resVec;

    std::transform( v.begin(), v.end(), std::back_inserter(resVec)
                  , []( const QString &s )
                    {
                        return s;
                    }
                  );

    return resVec;
}

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
#define IOA_QUADRIPLE_IMPL_STATIC_MAKE_FROM_VECTOR( typeName )           \
                static                                                   \
                typeName makeFrom( const QVector<QString> &lst )         \
                {                                                        \
                    typeName tmp;                                        \
                    tmp.from(lst);                                       \
                    return tmp;                                          \
                }

#define IOA_QUADRIPLE_IMPL_STATIC_MAKE_FROM_STRINGLIST( typeName )       \
                static                                                   \
                typeName makeFrom( const QStringList &lst )              \
                {                                                        \
                    return makeFrom(toStringVector(lst));                \
                }

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
struct QStringSingle
{
    QString first;

    void from( const QVector<QString> &lst )
    {
        int sz = lst.size();
        if (sz>=1) first = lst.at(0);
    }

    IOA_QUADRIPLE_IMPL_STATIC_MAKE_FROM_VECTOR(QStringSingle)
    IOA_QUADRIPLE_IMPL_STATIC_MAKE_FROM_STRINGLIST(QStringSingle)

}; // struct QStringSingle

//------------------------------
struct QStringPair
{
    QString first;
    QString second;

    void from( const QVector<QString> &lst )
    {
        int sz = lst.size();
        if (sz>=1) first  = lst.at(0);
        if (sz>=2) second = lst.at(1);
    }

    IOA_QUADRIPLE_IMPL_STATIC_MAKE_FROM_VECTOR(QStringPair)
    IOA_QUADRIPLE_IMPL_STATIC_MAKE_FROM_STRINGLIST(QStringPair)

}; // struct QstringPair

//------------------------------
struct QStringTriple
{
    QString first;
    QString second;
    QString third;

    void from( const QVector<QString> &lst )
    {
        int sz = lst.size();
        if (sz>=1) first  = lst.at(0);
        if (sz>=2) second = lst.at(1);
        if (sz>=3) third  = lst.at(2);
    }

    IOA_QUADRIPLE_IMPL_STATIC_MAKE_FROM_VECTOR(QStringTriple)
    IOA_QUADRIPLE_IMPL_STATIC_MAKE_FROM_STRINGLIST(QStringTriple)

}; // struct QStringTriple

//------------------------------
struct QStringQuatro
{
    QString first;
    QString second;
    QString third;
    QString fourth;

    void from( const QVector<QString> &lst )
    {
        int sz = lst.size();
        if (sz>=1) first  = lst.at(0);
        if (sz>=2) second = lst.at(1);
        if (sz>=3) third  = lst.at(2);
        if (sz>=4) fourth = lst.at(3);
    }

    IOA_QUADRIPLE_IMPL_STATIC_MAKE_FROM_VECTOR(QStringQuatro)
    IOA_QUADRIPLE_IMPL_STATIC_MAKE_FROM_STRINGLIST(QStringQuatro)

}; // struct QStringQuatro


//----------------------------------------------------------------------------
inline QStringList splitString( QString v, const QString &seps )
{
    for( QString::const_iterator it=seps.begin(); it!=seps.end(); ++it )
    {
        v.replace( *it, ";" );
    }

    return v.split( ';', Qt::SkipEmptyParts );
}

//----------------------------------------------------------------------------
inline QVector<QVector<QString> > splitString( QString v, const QString &recordSeps, const QString &itemSeps )
{
    QVector<QString> records = toStringVector(splitString( v, recordSeps ));

    QVector<QVector<QString> > resVec;

    std::transform( records.begin(), records.end(), std::back_inserter(resVec)
                  , [itemSeps]( const QString &s )
                    {
                        return toStringVector(splitString( s, itemSeps ));
                    }
                  );

    return resVec;
}

//----------------------------------------------------------------------------
inline
QVector<QVector<QString> > simpleSplitTo_ImplBase(const QString &str)
{
    return splitString( str, ";", ",.:" );
}

//----------------------------------------------------------------------------
template< typename CoupleType >
inline
QVector<CoupleType> simpleSplitTo( const QString &str )
{
    QVector<CoupleType> resVec;

    QVector<QVector<QString> > strsVec = simpleSplitTo_ImplBase(str);

    std::transform( strsVec.begin(), strsVec.end(), std::back_inserter(resVec)
                  , []( const QVector<QString> &v )
                    {
                        return CoupleType :: makeFrom(v);
                    }
                  );

    return resVec;
}

//----------------------------------------------------------------------------
inline std::set<QString> makeStdSet   (const QVector<QString> &v, bool bUpper = true )
{
    std::set<QString> resSet;

    QVector<QString>::const_iterator it = v.begin();
    for(; it != v.end(); ++it)
    {
        QString str = (bUpper ? it->toUpper() : *it);
        resSet.insert(str);
    }

    /*
    // Чёт не работает
    std::transform( v.begin(), v.end(), std::inserter(resSet)
                  , [bUpper]( const QString &v )
                    {
                        return bUpper ? v.toUpper() : v;
                    }
                  );
    */
    return resSet;
}

//----------------------------------------------------------------------------
inline QSet<QString>    makeSet   (const QVector<QString> &v, bool bUpper = true )
{
    std::set<QString> stdSet = makeStdSet(v, bUpper);
    return QSet<QString>( stdSet.begin(), stdSet.end() );
}

//----------------------------------------------------------------------------
inline QSet<QString>    makeSet   (const QStringList      &v, bool bUpper = true )
{
    return makeSet(makeVector(v, bUpper), false);
}

//----------------------------------------------------------------------------
inline QVector<QString> removeFirstItems(QVector<QString> v, int numItemsToRemove )
{
    if (numItemsToRemove>=v.size())
    {
        return QVector<QString>();
    }

    QVector<QString>::iterator eraseBegin = v.begin(), eraseEnd = v.begin();
    std::advance(eraseEnd, (std::size_t)(numItemsToRemove));

    v.erase(eraseBegin, eraseEnd);

    return v;
}

//----------------------------------------------------------------------------
inline int findItemByName( const QVector<QString> &v, const QString &str, int startIndex = 0 )
{
    for( int idx=startIndex; idx<v.size(); ++idx)
    {
        if (v[idx]==str)
            return idx;
    }

    return -1;
}

//----------------------------------------------------------------------------
inline QVector<QString> removeItemsByName( const QVector<QString> &v, const QString &name )
{
    QVector<QString> resVec;
    for( const auto &item : v )
    {
        if ( item!=name )
            resVec.push_back(item);
    }

    return resVec;
}

//----------------------------------------------------------------------------
inline QString mergeString(const QVector<QString> &v, const QString &sep )
{
    if (v.empty())
        return QString();

    if (v.size()==1)
        return v[0];

    QString resStr;

    QVector<QString>::const_iterator it = v.begin();

    resStr.append(*it); ++it;

    for( ; it!=v.end(); ++it)
    {
        resStr.append(sep);
        resStr.append(*it);
    }

    return resStr;
}

//----------------------------------------------------------------------------
inline QString mergeString(const QStringList &v, const QString &sep )
{
    return mergeString( toStringVector(v), sep );
}

//----------------------------------------------------------------------------
inline std::string mergeString(const std::vector<std::string> &v, const std::string &sep )
{
    if (v.empty())
        return std::string();

    if (v.size()==1)
        return v[0];

    std::string resStr;

    std::vector<std::string>::const_iterator it = v.begin();

    resStr.append(*it); ++it;

    for( ; it!=v.end(); ++it)
    {
        resStr.append(sep);
        resStr.append(*it);
    }

    return resStr;
}

/*
// Obsolete

inline
QList<QStringPair> simpleSplitToPairs( const QString &str )
{
    QList<QStringPair> resLst;

    QList<QStringList> lstOfLst = simpleSplitTo_ImplBase(str);

    QList<QStringList>::const_iterator it = lstOfLst.begin();
    for(; it != lstOfLst.end(); ++it)
    {
        QStringPair tmpPair;
        tmpPair.fromList(*it);
        resLst.push_back(tmpPair);
    }

    return resLst;
    
    #if 0

    QStringList lst = str.split(";");

    QStringList::const_iterator constIterator;
    for (constIterator = lst.constBegin(); constIterator != lst.constEnd(); ++constIterator)
    {
        QString strPair = *constIterator;
        strPair.replace(',', ":");
        strPair.replace('.', ":");

        QStringList p = strPair.split(":");

        int sz = p.size();

        if (!sz)
            continue;

        if (sz==1)
        {
            resLst.push_back( QStringPair{ p.at(0), QString() } );
            continue;
        }

        resLst.push_back( QStringPair{ p.at(0), p.at(1) } );
    }
    #endif

    return resLst;

}
*/


//----------------------------------------------------------------------------




} // namespace invest_openapi

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
#if defined(QT_SQL_LIB)
QString sqlEscape( const QSqlDatabase &db, const QString &str )
{
    QSqlField f(QLatin1String(""), QVariant::String);
    f.setValue(str);
    return db.driver()->formatValue(f);
}
#endif

//----------------------------------------------------------------------------



#include "instrument.h"


