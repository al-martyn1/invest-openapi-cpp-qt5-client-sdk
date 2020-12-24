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

#include "models.h"

#include "openapi_completable_future_base.h"
#include "openapi_completable_future.h"


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




//----------------------------------------------------------------------------
template< typename CurrencySourceType >
inline
Currency toCurrency(const CurrencySourceType &v)
{
    throw std::runtime_error("invest_openapi::toCurrency(const CurrencySourceType&) not implemented for this type");
}

//----------------------------------------------------------------------------
template< >
inline
Currency toCurrency<Currency>(const Currency &v)
{
    return v;
}

//----------------------------------------------------------------------------
template< >
inline
Currency toCurrency<Currency::eCurrency>(const Currency::eCurrency &v)
{
    Currency res;
    res.setValue(v);
    return res;
}

//----------------------------------------------------------------------------
template< >
inline
Currency toCurrency<QString>(const QString &v)
{
    Currency res;
    res.fromJson(v);
    return res;
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
template< typename InstrumentSourceType >
inline
InstrumentType toInstrumentType(const InstrumentSourceType &v)
{
    throw std::runtime_error("invest_openapi::toInstrumentType(const InstrumentSourceType&) not implemented for this type");
}

//----------------------------------------------------------------------------
template< >
inline
InstrumentType toInstrumentType<InstrumentType>(const InstrumentType &v)
{
    return v;
}

//----------------------------------------------------------------------------
template< >
inline
InstrumentType toInstrumentType<InstrumentType::eInstrumentType>(const InstrumentType::eInstrumentType &v)
{
    InstrumentType res;
    res.setValue(v);
    return res;
}

//----------------------------------------------------------------------------
template< >
inline
InstrumentType toInstrumentType<QString>(const QString &v)
{
    InstrumentType res;
    res.fromJson(v);
    return res;
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
template< typename InstrumentSourceType >
inline
CandleResolution toCandleResolution(const InstrumentSourceType &v)
{
    throw std::runtime_error("invest_openapi::toCandleResolution(const InstrumentSourceType&) not implemented for this type");
}

//----------------------------------------------------------------------------
template< >
inline
CandleResolution toCandleResolution<CandleResolution>(const CandleResolution &v)
{
    return v;
}

//----------------------------------------------------------------------------
template< >
inline
CandleResolution toCandleResolution<CandleResolution::eCandleResolution>(const CandleResolution::eCandleResolution &v)
{
    CandleResolution res;
    res.setValue(v);
    return res;
}

//----------------------------------------------------------------------------
template< >
inline
CandleResolution toCandleResolution<QString>(const QString &v)
{
    CandleResolution res;
    res.fromJson(v);
    return res;
}

//----------------------------------------------------------------------------





//----------------------------------------------------------------------------
template< typename OperationSourceType >
inline
OperationType toOperationType(const OperationSourceType &v)
{
    throw std::runtime_error("invest_openapi::toOperationType(const OperationSourceType&) not implemented for this type");
}

//----------------------------------------------------------------------------
template< >
inline
OperationType toOperationType<OperationType>(const OperationType &v)
{
    return v;
}

//----------------------------------------------------------------------------
template< >
inline
OperationType toOperationType<OperationType::eOperationType>(const OperationType::eOperationType &v)
{
    OperationType res;
    res.setValue(v);
    return res;
}

//----------------------------------------------------------------------------
template< >
inline
OperationType toOperationType<QString>(const QString &v)
{
    OperationType res;
    res.fromJson(v);
    return res;
}

//----------------------------------------------------------------------------











//----------------------------------------------------------------------------
template<typename MonetaryType>
inline
MonetaryType toMonetary( const QVariant &s )
{
    throw std::runtime_error("invest_openapi::toMonetary(QVariant) not implemented for this type");
}

//----------------------------------------------------------------------------
template<typename MonetaryType>
inline
MonetaryType toMonetary( const QString &s )
{
    throw std::runtime_error("invest_openapi::toMonetary(QVariant) not implemented for this type");
}

//----------------------------------------------------------------------------
template<typename MonetaryType>
inline
QVariant monetaryToVariant( const MonetaryType &m )
{
    throw std::runtime_error("invest_openapi::toVariant(MonetaryType) not implemented for this type");
}

//----------------------------------------------------------------------------
template<typename MonetaryType>
inline
QString monetaryToString( const MonetaryType &m )
{
    throw std::runtime_error("invest_openapi::toString(MonetaryType) not implemented for this type");
}





//----------------------------------------------------------------------------
template<>
inline
double toMonetary<double>( const QVariant &s )
{
    return s.toDouble();
}

//----------------------------------------------------------------------------
template<>
inline
double toMonetary<double>( const QString &s )
{
    QVariant qv = QVariant(s);
    return toMonetary<double>(qv);
}

//----------------------------------------------------------------------------
template<>
inline
QVariant monetaryToVariant<double>( const double &m )
{
    return QVariant(m);
}

//----------------------------------------------------------------------------
template<>
inline
QString monetaryToString<double>( const double &m )
{
    return QVariant(m).toString();
}

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
//! ������� false-��� ��� ������� ������� ������ getPayload() � �������
template< typename C, typename = void >
struct has_getPayload
  : std::false_type
{};

//------------------------------
//! �������������, ����������� ������� ������ getPayload() � �������
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
//! ������� ���������� payload ����� join'�
/*! 
     \tparam C ��� ����������
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


} // namespace invest_openapi






#include "instrument.h"
