/*! \file
    \brief 
 */

#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

namespace invest_openapi
{

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



} // namespace invest_openapi

