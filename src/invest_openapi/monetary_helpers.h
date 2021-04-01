#pragma once

#include <QObject>
#include <QVariant>
#include <QString>

#include "marty_decimal.h"


namespace invest_openapi
{

//----------------------------------------------------------------------------
template<typename MonetaryType, typename SourceType>
inline
MonetaryType toMonetary( const SourceType &s )
{
    throw std::runtime_error("invest_openapi::toMonetary(SourceType) not implemented for this type");
}

//----------------------------------------------------------------------------
template<typename TargetType, typename MonetaryType>
inline
TargetType fromMonetary( const MonetaryType &m )
{
    throw std::runtime_error("invest_openapi::fromMonetary(MonetaryType) not implemented for this type");
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
template<> inline  double          toMonetary   <double         ,  QVariant       >  ( const QVariant       &s )   { return s.toDouble(); }
template<> inline  double          toMonetary   <double         ,  QString        >  ( const QString        &s )   { return toMonetary<double, QVariant>( QVariant(s) ); }
template<> inline  double          toMonetary   <double         ,  double         >  ( const double         &s )   { return s; }
template<> inline  double          toMonetary   <double         ,  marty::Decimal >  ( const marty::Decimal &s )   { return (double)s; }

template<> inline  marty::Decimal  toMonetary   <marty::Decimal ,  QVariant       >  ( const QVariant       &s )   { return marty::Decimal::fromString(s.toString().toStdString()); }
template<> inline  marty::Decimal  toMonetary   <marty::Decimal ,  QString        >  ( const QString        &s )   { return marty::Decimal::fromString(s.toStdString()); }
template<> inline  marty::Decimal  toMonetary   <marty::Decimal ,  double         >  ( const double         &s )   { return marty::Decimal(s); }
template<> inline  marty::Decimal  toMonetary   <marty::Decimal ,  marty::Decimal >  ( const marty::Decimal &s )   { return s; }

template<> inline  QVariant        fromMonetary <QVariant       ,  double         >  ( const double &m )           { return QVariant(m); }
template<> inline  QString         fromMonetary <QString        ,  double         >  ( const double &m )           { return QVariant(m).toString(); }
template<> inline  double          fromMonetary <double         ,  double         >  ( const double &m )           { return m; }
template<> inline  marty::Decimal  fromMonetary <marty::Decimal ,  double         >  ( const double &m )           { return marty::Decimal(m); }

//template<> inline  QVariant        fromMonetary <QVariant       ,  marty::Decimal >  ( const marty::Decimal &m )   { return QVariant(QString::fromStdString(marty::Decimal::toString(m))); }
template<> inline  QVariant        fromMonetary <QVariant       ,  marty::Decimal >  ( const marty::Decimal &m )   { return QVariant(QString::fromStdString(marty::Decimal(m).toString())); }
//template<> inline  QString         fromMonetary <QString        ,  marty::Decimal >  ( const marty::Decimal &m )   { return QString::fromStdString(marty::Decimal::toString(m)); }
template<> inline  QString         fromMonetary <QString        ,  marty::Decimal >  ( const marty::Decimal &m )   { return QString::fromStdString(marty::Decimal().toString()); }
template<> inline  double          fromMonetary <double         ,  marty::Decimal >  ( const marty::Decimal &m )   { return (double)m; }
template<> inline  marty::Decimal  fromMonetary <marty::Decimal ,  marty::Decimal >  ( const marty::Decimal &m )   { return m; }

//----------------------------------------------------------------------------



} // namespace invest_openapi

