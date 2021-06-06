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

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
template< typename ValType >
inline
OutlierLimits< ValType >
getPercentiles( const std::vector< ValType >& sortedVals, std::size_t percentile = 25 ) //!< 25 for quartiles
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
template< typename ValType >
inline
OutlierLimits< ValType >
calcOutlierLimits( const OutlierLimits< ValType > &limits )
{
    ValType percentileLow   = limits.lower;
    ValType percentileHight = limits.upper;

    ValType deltaPercentile       = percentileHight - percentileLow;
    ValType deltaPercentileScaled = 3*deltaPercentile / 2;

    ValType limLow   = percentileLow   - deltaPercentileScaled;
    ValType limHight = percentileHight + deltaPercentileScaled;

    if (limLow<0)
        limLow = 0;

    return OutlierLimits< ValType >{  /* limLow */ 2*percentileLow/3, limHight };
}

//----------------------------------------------------------------------------
//! Вычисляем текущую цену инструмента
/*!
    Текущей ценой считается цена последней сделки. 
    Но у нас нет возможности её получить.
    Максимально оперативно обновляется стакан.
    Поэтому текущей ценой будем считать среднее между макс предложением (bid'ом) и спросом (ask'ом),
    приведеное к шкале цен инструмента (к ближайшему делящемуся на price increment).

*/

inline
marty::Decimal calcInstrumentPrice( marty::Decimal p1, marty::Decimal p2, const marty::Decimal &priceStep, bool roundDown = false )
{
    if (p1 > p2)
        p1.swap(p2);

    // Here p1 is less than p2


    using marty::Decimal;

    Decimal avg = (p1+p2) / marty::Decimal(2);

    //Decimal priceStepDivRawRes = avg.mod_helper_raw_div( priceStep );
    Decimal finalPriceMod      = avg.mod_helper( priceStep );

    Decimal finalPrice         = finalPriceMod * priceStep;
    
    Decimal finalPriceMin      = finalPrice;
    Decimal finalPriceMax      = finalPrice + priceStep;


    // Decimal deltaToLower = finalPriceMax - p1;
    // Decimal deltaToUpper = p2 - finalPriceMin;

    Decimal deltaToLower = finalPrice - p1;
    Decimal deltaToUpper = p2 - finalPrice;

    int cmpResult = deltaToLower.compare( deltaToUpper );
    if (cmpResult==0)
    {
        if (roundDown)
            return finalPriceMin;
        else
            return finalPriceMax;
    }
    else if (cmpResult<=0)
    {
        return finalPriceMin;
    }
    else
    {
        return finalPriceMax;
    }

}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
} // namespace invest_openapi


//----------------------------------------------------------------------------
