#pragma once

#include <QObject>
#include <QDebug>
#include <QtDebug>
#include <QTest>
#include <QTimeZone>

//#include <QtConcurrent/QtConcurrent>

#include <iostream>
#include <exception>
#include <stdexcept>

#include "models.h"


inline
QDebug& operator<<( QDebug& d, const OpenAPI::Order &v )
{
    d.nospace().noquote() << v.asJson();
    return d;
}

inline
QDebug& operator<<( QDebug& d, const OpenAPI::PlacedLimitOrder &v )
{
    d.nospace().noquote() << v.asJson();
    return d;
}

inline
QDebug& operator<<( QDebug& d, const OpenAPI::PlacedMarketOrder &v )
{
    d.nospace().noquote() << v.asJson();
    return d;
}

