/*! \file
    \brief 
 */

#pragma once

#include <exception>
#include <stdexcept>
#include <map>

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
inline
QString listStringNormalize( QString s )
{
    s.replace(':', ";"); // replace *nix style list separator to windows style separator
    s.replace(',', ";"); // replace commas to ';' (windows style separator)
    return s;
}

//----------------------------------------------------------------------------
inline
QStringList listStringSplit( QString s )
{
    s = listStringNormalize(s);
    return s.split( ';', Qt::SkipEmptyParts );
}

//----------------------------------------------------------------------------
inline
QStringList pathListSplit( QString pathList )
{
    pathList.replace(':', ";"); // replace *nix style list separator to windows style separator
    return pathList.split( ';', Qt::SkipEmptyParts );
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
#define IOA_QUADRIPLE_IMPL_STATIC_MAKE_FROM_LIST( typeName )     \
                static                                           \
                typeName makeFromList( const QStringList &lst )  \
                {                                                \
                    typeName tmp;                                \
                    tmp.fromList(lst);                           \
                    return tmp;                                  \
                }


//----------------------------------------------------------------------------
struct QStringSingle
{
    QString first;

    void fromList( const QStringList &lst )
    {
        int sz = lst.size();
        if (sz>=1) first = lst.at(0);
    }

    IOA_QUADRIPLE_IMPL_STATIC_MAKE_FROM_LIST(QStringSingle)

}; // struct QStringSingle

//------------------------------
struct QStringPair
{
    QString first;
    QString second;

    void fromList( const QStringList &lst )
    {
        int sz = lst.size();
        if (sz>=1) first  = lst.at(0);
        if (sz>=2) second = lst.at(1);
    }

    IOA_QUADRIPLE_IMPL_STATIC_MAKE_FROM_LIST(QStringPair)

}; // struct QstringPair

//------------------------------
struct QStringTriple
{
    QString first;
    QString second;
    QString third;

    void fromList( const QStringList &lst )
    {
        int sz = lst.size();
        if (sz>=1) first  = lst.at(0);
        if (sz>=2) second = lst.at(1);
        if (sz>=3) third  = lst.at(2);
    }

    IOA_QUADRIPLE_IMPL_STATIC_MAKE_FROM_LIST(QStringTriple)

}; // struct QStringTriple

//------------------------------
struct QStringQuatro
{
    QString first;
    QString second;
    QString third;
    QString fourth;

    void fromList( const QStringList &lst )
    {
        int sz = lst.size();
        if (sz>=1) first  = lst.at(0);
        if (sz>=2) second = lst.at(1);
        if (sz>=3) third  = lst.at(2);
        if (sz>=4) fourth = lst.at(3);
    }

    IOA_QUADRIPLE_IMPL_STATIC_MAKE_FROM_LIST(QStringQuatro)

}; // struct QStringQuatro

//----------------------------------------------------------------------------
inline
QList<QStringList> simpleSplitTo_ImplBase(const QString &str)
{
    QStringList         lst = str.split(";");

    QList<QStringList>  res;

    QStringList::const_iterator constIterator;
    for (constIterator = lst.constBegin(); constIterator != lst.constEnd(); ++constIterator)
    {
        QString strList = *constIterator;
        strList.replace(',', ":");
        strList.replace('.', ":");

        res.push_back( strList.split(":") );
    }

    return res;
}

//----------------------------------------------------------------------------
template< typename CoupleType >
inline
QList<CoupleType> simpleSplitTo( const QString &str )
{
    QList<CoupleType> resLst;

    QList<QStringList> lstOfLst = simpleSplitTo_ImplBase(str);

    QList<QStringList>::const_iterator it = lstOfLst.begin();
    for(; it != lstOfLst.end(); ++it)
    {
        /*
        CoupleType tmpPair;
        tmpPair.fromList(*it);
        resLst.push_back(tmpPair);
        */

        resLst.push_back(CoupleType::makeFromList(*it));
        
    }

    return resLst;
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


