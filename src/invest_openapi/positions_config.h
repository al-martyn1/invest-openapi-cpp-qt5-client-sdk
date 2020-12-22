/*! \file
    \brief 
 */

#pragma once

#include <QString>
#include <QSettings>
#include <QVector>
#include <QStringList>

#include <exception>
#include <stdexcept>

#include "models.h"

#include "utility.h"

namespace invest_openapi
{


inline
QVector<SandboxSetPositionBalanceRequest> readSandboxPositionsConfig( const QSettings &settings, bool readStrict = false )
{
    QVector<SandboxSetPositionBalanceRequest> resVec;

    QString positionsGroupName = "sandbox.positions";
    QString positionFigis = settings.value(positionsGroupName).toString();
    QStringList positionsList = listStringSplit(positionFigis);

    for( const auto &positionFigi : positionsList )
    {
        auto positionTrimmedFigi = positionFigi.trimmed();
        if (positionTrimmedFigi.isEmpty())
            continue;

        QString positionParamName = positionsGroupName + QString(".") + positionTrimmedFigi;

        QVariant positionBalanceQv = settings.value(positionParamName);
        if (!positionBalanceQv.isValid() || positionBalanceQv.isNull())
        {
            if (readStrict)
                throw std::runtime_error("Missing position balance");
        }

        SandboxSetPositionBalanceRequest positionBalance;
        positionBalance.setFigi(positionTrimmedFigi);
        positionBalance.setBalance( positionBalanceQv.toDouble() );

        resVec.push_back(positionBalance);
    }

    return resVec;
}


} // namespace invest_openapi

