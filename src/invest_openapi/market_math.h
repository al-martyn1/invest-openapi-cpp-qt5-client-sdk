#pragma once

#include "marty_decimal.h"
#include "undef_min_max.h"



//----------------------------------------------------------------------------
/*
    Links

        Выброс (статистика) - https://ru.wikipedia.org/wiki/%D0%92%D1%8B%D0%B1%D1%80%D0%BE%D1%81_(%D1%81%D1%82%D0%B0%D1%82%D0%B8%D1%81%D1%82%D0%B8%D0%BA%D0%B0)
        Квартет Энскомба    - https://ru.wikipedia.org/wiki/%D0%9A%D0%B2%D0%B0%D1%80%D1%82%D0%B5%D1%82_%D0%AD%D0%BD%D1%81%D0%BA%D0%BE%D0%BC%D0%B1%D0%B0
        Корреляция          - https://ru.wikipedia.org/wiki/%D0%9A%D0%BE%D1%80%D1%80%D0%B5%D0%BB%D1%8F%D1%86%D0%B8%D1%8F

        Критерий Шовене     - https://en.wikipedia.org/wiki/Chauvenet%27s_criterion
        Критерий Граббса    - https://ru.wikipedia.org/wiki/%D0%9A%D1%80%D0%B8%D1%82%D0%B5%D1%80%D0%B8%D0%B9_%D0%93%D1%80%D0%B0%D0%B1%D0%B1%D1%81%D0%B0
        Критерий Пирса      - https://en.wikipedia.org/wiki/Peirce%27s_criterion
        Критерий Диксона    - https://en.wikipedia.org/wiki/Dixon%27s_Q_test

        Межквартильное расстояние - https://en.wikipedia.org/wiki/Interquartile_range
            (X25 - 1.5*(X75-X25)) , ( X75 + 1.5*(X75-X25) )

        Квантиль - https://ru.wikipedia.org/wiki/%D0%9A%D0%B2%D0%B0%D0%BD%D1%82%D0%B8%D0%BB%D1%8C
              


*/




//----------------------------------------------------------------------------
namespace invest_openapi
{

//----------------------------------------------------------------------------
template< typename ValType >
struct OutlierLimits
{
    ValType    lower; //!< All which is <= lower is an outlier
    ValType    upper; //!< All which is >= upper is an outlier
};



template< typename ValType >
inline
OutlierLimits< ValType >
calcOutlierLimits( const std::vector< ValType >& sortedVals, std::size_t percentile )
{
    if (percentile>50)
        percentile = 50;

    std::size_t idxLow  = sortedVals.size()*percentile / 100;
    if (idxLow>=sortedVals.size())
        return OutlierLimits< ValType >{ ValType(0), ValType(0) };

    std::size_t idxHigh = sortedVals.size()-idxLow;

    return OutlierLimits< ValType >{ sortedVals[idxLow], sortedVals[idxHigh] };
}





//----------------------------------------------------------------------------
} // namespace invest_openapi


//----------------------------------------------------------------------------
