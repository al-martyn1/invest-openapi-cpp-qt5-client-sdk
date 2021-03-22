/*! \file
    \brief Decimal type test

 */

#include <iostream>
#include <exception>
#include <stdexcept>
#include <sstream>

#include "invest_openapi/config_helpers.h"
#include "invest_openapi/api_config.h"
#include "invest_openapi/auth_config.h"
#include "invest_openapi/currencies_config.h"

#include "invest_openapi/invest_openapi.h"
#include "invest_openapi/factory.h"
#include "invest_openapi/openapi_completable_future.h"

#include "invest_openapi/marty_decimal.h"



INVEST_OPENAPI_MAIN()
{
    using std::cout;
    using std::endl;

    using marty::Decimal;
    using marty::DecimalPrecision;
    //using marty::toString;
    //using marty::fromString;

    unsigned totalCtorTests       = 0;
    unsigned totalCtorTestsFailed = 0;


    #define DECIMAL_CTOR_TEST( val, strResForCompare )                                      \
                do                                                                          \
                {                                                                           \
                    ++totalCtorTests;                                                       \
                    Decimal     decimal    = Decimal(val);                                  \
                    std::string strDecimal = decimal.toString( /* precision - auto */ );    \
                                                                                            \
                    bool bGood /* Johny */ = decimal.checkIsExact(strResForCompare);        \
                    if (!bGood)                                                             \
                       ++totalCtorTestsFailed;                                              \
                                                                                            \
                                                                                            \
                    cout << "[" << (bGood ? "+" : "-") << "]  " << (decimal<0 ? "" : " ") << val << " converted to Decimal is '" << strDecimal << "'"; \
                    if (!bGood)                                                             \
                    {                                                                       \
                        cout << " (expected '" << strResForCompare << "')";                 \
                    }                                                                       \
                    cout << endl;                                                           \
                                                                                            \
                } while(0)

    #define DECIMAL_CTOR_TEST_WITH_PREC( val, prec, strResForCompare )                      \
                do                                                                          \
                {                                                                           \
                    ++totalCtorTests;                                                       \
                    Decimal     decimal    = Decimal(val, marty::DecimalPrecision(prec));   \
                    std::string strDecimal = decimal.toString( /* precision - auto */ );    \
                                                                                            \
                    bool bGood /* Johny */ = decimal.checkIsExact(strResForCompare);        \
                    if (!bGood)                                                             \
                       ++totalCtorTestsFailed;                                              \
                                                                                            \
                                                                                            \
                    cout << "[" << (bGood ? "+" : "-") << "]  " << (decimal<0 ? "" : " ") << val << " converted to Decimal with precision " << prec << " is '" << strDecimal << "'"; \
                    if (!bGood)                                                             \
                    {                                                                       \
                        cout << " (expected '" << strResForCompare << "')";                 \
                    }                                                                       \
                    cout << endl;                                                           \
                                                                                            \
                } while(0)


    DECIMAL_CTOR_TEST( (std::uint64_t)3    ,       "3" );
    DECIMAL_CTOR_TEST( (std::int64_t )7    ,       "7" );
    DECIMAL_CTOR_TEST( (std::int64_t )-10  ,     "-10" );
    DECIMAL_CTOR_TEST( "123.01"            ,     "123.01" );
    DECIMAL_CTOR_TEST( "13.101"            ,     "13.101" );
    DECIMAL_CTOR_TEST( "3.1415"            ,     "3.1415" );
    DECIMAL_CTOR_TEST( 3.1415              ,     "3.1415" );
    DECIMAL_CTOR_TEST( -3.1415             ,    "-3.1415" );
    DECIMAL_CTOR_TEST( -100.10101          ,  "-100.10101" );
    //DECIMAL_CTOR_TEST(  3.1415926535897932  ,  "3.1415926535897932" );
    DECIMAL_CTOR_TEST(  3.1415926          ,  "3.141593" );
    // DECIMAL_CTOR_TEST();

    Decimal decimal01    = Decimal(((std::uint64_t)3141592654ULL), marty::DecimalPrecision(9));

    // 3.141592654
    //                                             .
    DECIMAL_CTOR_TEST_WITH_PREC(  ((std::uint64_t)3141592654ULL), 9, "3.141592654" );


    cout << endl;
    cout << "------------------------------" << endl;
    cout << endl;

    cout << "Failed " << totalCtorTestsFailed << " CTOR tests from total " << totalCtorTestsFailed << endl;
    if (!totalCtorTestsFailed)
        cout << "+++ All CTOR tests passed"  << endl;

    cout << endl;
    cout << endl;



    #define DECIMAL_OP_TEST( val1, val2, op, strResForCompare )                             \
                do                                                                          \
                {                                                                           \
                    ++totalOpTests;                                                         \
                    Decimal     d1       = Decimal(val1);                                   \
                    Decimal     d2       = Decimal(val2);                                   \
                    Decimal     dRes     = d1 op d2;                                        \
                    std::string strRes   = dRes.toString( /* precision - auto */ );         \
                                                                                            \
                    bool bGood /* Johny */ = dRes.checkIsExact(strResForCompare);           \
                    if (!bGood)                                                             \
                       ++totalOpTestsFailed;                                                \
                                                                                            \
                                                                                            \
                    cout << "[" << (bGood ? "+" : "-") << "]  " << d1 << " " << #op << " " << d2 << " = " << strRes << ""; \
                    if (!bGood)                                                             \
                    {                                                                       \
                        cout << " (expected '" << strResForCompare << "')";                 \
                    }                                                                       \
                    cout << endl;                                                           \
                                                                                            \
                } while(0)


    unsigned totalOpTests       = 0;
    unsigned totalOpTestsFailed = 0;

    DECIMAL_OP_TEST( 3, 4.12, +, "7.12" );


    cout << endl;
    cout << "------------------------------" << endl;
    cout << endl;

    cout << "Failed " << totalOpTestsFailed << " OP tests from total " << totalOpTestsFailed << endl;
    if (!totalOpTestsFailed)
        cout << "+++ All OP tests passed"  << endl;

    cout << endl;
    cout << endl;


    /*
    #define PRINT( var ) cout << #var << ": "<< var << endl;

    Decimal d1_u64_3   = (std::uint64_t)(3)  ;     PRINT(d1_u64_3);
    Decimal d1_i64_7   = (std::int64_t )(7)  ;     PRINT(d1_i64_7);
    Decimal d1_i64_m10 = (std::int64_t )(-10);     PRINT(d1_i64_m10);
    Decimal d1_i64_15_000 = Decimal(15, DecimalPrecision(3)); PRINT(d1_i64_15_000);
    Decimal d1 = Decimal::fromString("123.01");    PRINT(d1);
    Decimal d2 = Decimal::fromString("13.101");    PRINT(d2);
    Decimal d1_2_sum = d1 + d2;                    PRINT(d1_2_sum);  // 123.01 + 13.101 =  136.111
    Decimal d1_2_raz = d1 - d2;                    PRINT(d1_2_raz);  // 123.01 - 13.101 =  109.909
    Decimal d1_2_mul = d1 * d2;                    PRINT(d1_2_mul);  // 123.01 * 13.101 = 1611.55401
    Decimal d1_2_div = d1 / d2;                    PRINT(d1_2_div);  // 123.01 / 13.101 =    9.389359590870925883520341958629112281505228608503167697122357071979238226089611480039691626593389817571177772689107701702160140447294099687
    Decimal d1_2_ost = d1 % d2;                    PRINT(d1_2_ost);  // 123.01 % 13.101 =    5.101

    auto    d1_2_sum_plus_15_a = d1_2_sum + (std::int64_t )(15); PRINT(d1_2_sum_plus_15_a); // 136.111 + 15 = 151.111
    auto    d1_2_sum_plus_15_b = d1_2_sum + (std::uint64_t)(15); PRINT(d1_2_sum_plus_15_b);
    auto    d1_2_sum_plus_15_c = d1_2_sum + (int          )(15); PRINT(d1_2_sum_plus_15_c);
    auto    d1_2_sum_plus_15_d = d1_2_sum + (unsigned     )(15); PRINT(d1_2_sum_plus_15_d);
    auto    d1_2_sum_plus_15_e = d1_2_sum + (double       )(15); PRINT(d1_2_sum_plus_15_e);


    Decimal divideD1   = Decimal( 100, DecimalPrecision(2) );
    Decimal divideD2   = Decimal(  25, DecimalPrecision(1) );
    Decimal divideRes  = divideD1.divide(divideD2, 6);
    
    cout << divideRes.toString() << endl;
    */




    unsigned totalRoundingTests       = 0;
    unsigned totalRoundingTestsFailed = 0;


    #define ROUNDING_TEST( dblVal, strResForCompare, roundingPrecision, roundingMethod ) \
                do                                                                       \
                {                                                                        \
                    ++totalRoundingTests;                                                \
                    Decimal decimal    = Decimal(dblVal);                                \
                    Decimal roundedVal = decimal.rounded( roundingPrecision, Decimal::RoundingMethod::roundingMethod ); \
                    /*std::ostringstream os;*/                                           \
                    std::string strRes = roundedVal.toString(roundingPrecision);         \
                                                                                         \
                    bool bGood /* Johny */ = decimal.checkIsExact(strResForCompare);        \
                    if (!bGood)                                                          \
                       ++totalRoundingTestsFailed;                                       \
                                                                                         \
                                                                                         \
                    cout << "[" << (bGood ? "+" : "-") << "]  " << (decimal<0 ? "" : " ") << decimal.toString(roundingPrecision+1) << " rounded to " << roundingPrecision << " signs with " << #roundingMethod << " rounding method is " << strRes; \
                    if (!bGood)                                                          \
                    {                                                                    \
                        cout << " (expected " << strResForCompare << ")";                \
                    }                                                                    \
                    cout << endl;                                                        \
                                                                                         \
                } while(0)


    // https://en.wikipedia.org/wiki/Rounding#Round_half_to_odd

    cout << endl;
    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.0  ,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.01 ,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.001,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.112,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.49 ,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.499,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.5  ,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.51 ,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.501,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.511,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.69 ,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.699,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.7  ,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.71 ,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.701,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.911,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.999,  "23", 0, roundFloor           );
    ROUNDING_TEST(   24.0  ,  "24", 0, roundFloor           );
    ROUNDING_TEST(   24.01 ,  "24", 0, roundFloor           );
    ROUNDING_TEST(   24.001,  "24", 0, roundFloor           );

    cout << endl;
    ROUNDING_TEST(  -23.0  , "-23", 0, roundFloor           );
    ROUNDING_TEST(  -23.01 , "-24", 0, roundFloor           );
    ROUNDING_TEST(  -23.001, "-24", 0, roundFloor           );
    ROUNDING_TEST(  -23.112, "-24", 0, roundFloor           );
    ROUNDING_TEST(  -23.2  , "-24", 0, roundFloor           );
    ROUNDING_TEST(  -23.21 , "-24", 0, roundFloor           );
    ROUNDING_TEST(  -23.49 , "-24", 0, roundFloor           );
    ROUNDING_TEST(  -23.499, "-24", 0, roundFloor           );
    ROUNDING_TEST(  -23.5  , "-24", 0, roundFloor           );
    ROUNDING_TEST(  -23.51 , "-24", 0, roundFloor           );
    ROUNDING_TEST(  -23.501, "-24", 0, roundFloor           );
    ROUNDING_TEST(  -23.511, "-24", 0, roundFloor           );
    ROUNDING_TEST(  -23.911, "-24", 0, roundFloor           );
    ROUNDING_TEST(  -23.999, "-24", 0, roundFloor           );
    ROUNDING_TEST(  -24.0  , "-24", 0, roundFloor           );
    ROUNDING_TEST(  -24.01 , "-25", 0, roundFloor           );
    ROUNDING_TEST(  -24.011, "-25", 0, roundFloor           );

    cout << endl;
    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.0  ,  "23", 0, roundCeil            );
    ROUNDING_TEST(   23.01 ,  "24", 0, roundCeil            );
    ROUNDING_TEST(   23.001,  "24", 0, roundCeil            );
    ROUNDING_TEST(   23.2  ,  "24", 0, roundCeil            );
    ROUNDING_TEST(   23.49 ,  "24", 0, roundCeil            );
    ROUNDING_TEST(   23.499,  "24", 0, roundCeil            );
    ROUNDING_TEST(   23.5  ,  "24", 0, roundCeil            );
    ROUNDING_TEST(   23.51 ,  "24", 0, roundCeil            );
    ROUNDING_TEST(   23.501,  "24", 0, roundCeil            );
    ROUNDING_TEST(   23.999,  "24", 0, roundCeil            );
    ROUNDING_TEST(   24.0  ,  "24", 0, roundCeil            );
    ROUNDING_TEST(   24.001,  "25", 0, roundCeil            );

    cout << endl;
    ROUNDING_TEST(  -23.0  , "-23", 0, roundCeil            );
    ROUNDING_TEST(  -23.01 , "-23", 0, roundCeil            );
    ROUNDING_TEST(  -23.001, "-23", 0, roundCeil            );
    ROUNDING_TEST(  -23.49 , "-23", 0, roundCeil            );
    ROUNDING_TEST(  -23.499, "-23", 0, roundCeil            );
    ROUNDING_TEST(  -23.5  , "-23", 0, roundCeil            );
    ROUNDING_TEST(  -23.51 , "-23", 0, roundCeil            );
    ROUNDING_TEST(  -23.501, "-23", 0, roundCeil            );
    ROUNDING_TEST(  -23.7  , "-23", 0, roundCeil            );
    ROUNDING_TEST(  -23.701, "-23", 0, roundCeil            );
    ROUNDING_TEST(  -23.999, "-23", 0, roundCeil            );
    ROUNDING_TEST(  -24.0  , "-24", 0, roundCeil            );
    ROUNDING_TEST(  -24.01 , "-24", 0, roundCeil            );

    cout << endl;
    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.0  ,  "23", 0, roundTrunc           );
    ROUNDING_TEST(   23.01 ,  "23", 0, roundTrunc           );
    ROUNDING_TEST(   23.49 ,  "23", 0, roundTrunc           );
    ROUNDING_TEST(   23.499,  "23", 0, roundTrunc           );
    ROUNDING_TEST(   23.5  ,  "23", 0, roundTrunc           );
    ROUNDING_TEST(   23.51 ,  "23", 0, roundTrunc           );
    ROUNDING_TEST(   23.7  ,  "23", 0, roundTrunc           );
    ROUNDING_TEST(   23.99 ,  "23", 0, roundTrunc           );
    ROUNDING_TEST(   24.0  ,  "24", 0, roundTrunc           );
    ROUNDING_TEST(   24.01 ,  "24", 0, roundTrunc           );

    cout << endl;
    ROUNDING_TEST(  -23.0  , "-23", 0, roundTrunc           );
    ROUNDING_TEST(  -23.01 , "-23", 0, roundTrunc           );
    ROUNDING_TEST(  -23.49 , "-23", 0, roundTrunc           );
    ROUNDING_TEST(  -23.499, "-23", 0, roundTrunc           );
    ROUNDING_TEST(  -23.5  , "-23", 0, roundTrunc           );
    ROUNDING_TEST(  -23.51 , "-23", 0, roundTrunc           );
    ROUNDING_TEST(  -23.7  , "-23", 0, roundTrunc           );
    ROUNDING_TEST(  -23.99 , "-23", 0, roundTrunc           );
    ROUNDING_TEST(  -24.0  , "-24", 0, roundTrunc           );
    ROUNDING_TEST(  -24.01 , "-24", 0, roundTrunc           );

    cout << endl;
    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.0  ,  "23", 0, roundTowardsInf      );
    ROUNDING_TEST(   23.01 ,  "24", 0, roundTowardsInf      );
    ROUNDING_TEST(   23.11 ,  "24", 0, roundTowardsInf      );
    ROUNDING_TEST(   23.2  ,  "24", 0, roundTowardsInf      );
    ROUNDING_TEST(   23.49 ,  "24", 0, roundTowardsInf      );
    ROUNDING_TEST(   23.499,  "24", 0, roundTowardsInf      );
    ROUNDING_TEST(   23.5  ,  "24", 0, roundTowardsInf      );
    ROUNDING_TEST(   23.51 ,  "24", 0, roundTowardsInf      );
    ROUNDING_TEST(   23.7  ,  "24", 0, roundTowardsInf      );
    ROUNDING_TEST(   23.99 ,  "24", 0, roundTowardsInf      );
    ROUNDING_TEST(   24.0  ,  "24", 0, roundTowardsInf      );
    ROUNDING_TEST(   24.01 ,  "25", 0, roundTowardsInf      );

    cout << endl;
    ROUNDING_TEST(  -23.0  , "-23", 0, roundTowardsInf      );
    ROUNDING_TEST(  -23.01 , "-24", 0, roundTowardsInf      );
    ROUNDING_TEST(  -23.2  , "-24", 0, roundTowardsInf      );
    ROUNDING_TEST(  -23.49 , "-24", 0, roundTowardsInf      );
    ROUNDING_TEST(  -23.499, "-24", 0, roundTowardsInf      );
    ROUNDING_TEST(  -23.5  , "-24", 0, roundTowardsInf      );
    ROUNDING_TEST(  -23.5  , "-24", 0, roundTowardsInf      );
    ROUNDING_TEST(  -23.51 , "-24", 0, roundTowardsInf      );
    ROUNDING_TEST(  -23.501, "-24", 0, roundTowardsInf      );
    ROUNDING_TEST(  -23.7  , "-24", 0, roundTowardsInf      );
    ROUNDING_TEST(  -23.99 , "-24", 0, roundTowardsInf      );
    ROUNDING_TEST(  -24.0  , "-24", 0, roundTowardsInf      );
    ROUNDING_TEST(  -24.01 , "-25", 0, roundTowardsInf      );

    cout << endl;
    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.0  ,  "23", 0, roundHalfUp          );
    ROUNDING_TEST(   23.1  ,  "23", 0, roundHalfUp          );
    ROUNDING_TEST(   23.01 ,  "23", 0, roundHalfUp          );
    ROUNDING_TEST(   23.3  ,  "23", 0, roundHalfUp          );
    ROUNDING_TEST(   23.49 ,  "23", 0, roundHalfUp          );
    ROUNDING_TEST(   23.499,  "23", 0, roundHalfUp          );
    ROUNDING_TEST(   23.5  ,  "24", 0, roundHalfUp          );
    ROUNDING_TEST(   23.51 ,  "24", 0, roundHalfUp          );
    ROUNDING_TEST(   23.501,  "24", 0, roundHalfUp          );
    ROUNDING_TEST(   23.7  ,  "24", 0, roundHalfUp          );
    ROUNDING_TEST(   23.9  ,  "24", 0, roundHalfUp          );
    ROUNDING_TEST(   23.99 ,  "24", 0, roundHalfUp          );
    ROUNDING_TEST(   24.0  ,  "24", 0, roundHalfUp          );
    ROUNDING_TEST(   24.01 ,  "24", 0, roundHalfUp          );

    cout << endl;
    ROUNDING_TEST(  -23.0  , "-23", 0, roundHalfUp          );
    ROUNDING_TEST(  -23.1  , "-23", 0, roundHalfUp          );
    ROUNDING_TEST(  -23.49 , "-23", 0, roundHalfUp          );
    ROUNDING_TEST(  -23.499, "-23", 0, roundHalfUp          );
    ROUNDING_TEST(  -23.5  , "-23", 0, roundHalfUp          );
    ROUNDING_TEST(  -23.51 , "-24", 0, roundHalfUp          );
    ROUNDING_TEST(  -23.501, "-24", 0, roundHalfUp          );
    ROUNDING_TEST(  -23.9  , "-24", 0, roundHalfUp          );
    ROUNDING_TEST(  -23.99 , "-24", 0, roundHalfUp          );
    ROUNDING_TEST(  -24.0  , "-24", 0, roundHalfUp          );
    ROUNDING_TEST(  -24.01 , "-24", 0, roundHalfUp          );

    cout << endl;
    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.0  ,  "23", 0, roundHalfDown        );
    ROUNDING_TEST(   23.1  ,  "23", 0, roundHalfDown        );
    ROUNDING_TEST(   23.3  ,  "23", 0, roundHalfDown        );
    ROUNDING_TEST(   23.49 ,  "23", 0, roundHalfDown        );
    ROUNDING_TEST(   23.499,  "23", 0, roundHalfDown        );
    ROUNDING_TEST(   23.5  ,  "23", 0, roundHalfDown        );
    ROUNDING_TEST(   23.51 ,  "24", 0, roundHalfDown        );
    ROUNDING_TEST(   23.501,  "24", 0, roundHalfDown        );
    ROUNDING_TEST(   23.9  ,  "24", 0, roundHalfDown        );
    ROUNDING_TEST(   23.99 ,  "24", 0, roundHalfDown        );
    ROUNDING_TEST(   24.0  ,  "24", 0, roundHalfDown        );
    ROUNDING_TEST(   24.01 ,  "24", 0, roundHalfDown        );

    cout << endl;
    ROUNDING_TEST(  -23.0  , "-23", 0, roundHalfDown        );
    ROUNDING_TEST(  -23.1  , "-23", 0, roundHalfDown        );
    ROUNDING_TEST(  -23.3  , "-23", 0, roundHalfDown        );
    ROUNDING_TEST(  -23.49 , "-23", 0, roundHalfDown        );
    ROUNDING_TEST(  -23.499, "-23", 0, roundHalfDown        );
    ROUNDING_TEST(  -23.5  , "-24", 0, roundHalfDown        );
    ROUNDING_TEST(  -23.51 , "-24", 0, roundHalfDown        );
    ROUNDING_TEST(  -23.501, "-24", 0, roundHalfDown        );
    ROUNDING_TEST(  -23.9  , "-24", 0, roundHalfDown        );
    ROUNDING_TEST(  -23.99 , "-24", 0, roundHalfDown        );
    ROUNDING_TEST(  -24.0  , "-24", 0, roundHalfDown        );
    ROUNDING_TEST(  -24.1  , "-24", 0, roundHalfDown        );

    cout << endl;
    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.0  ,  "23", 0, roundHalfTowardsZero );
    ROUNDING_TEST(   23.1  ,  "23", 0, roundHalfTowardsZero );
    ROUNDING_TEST(   23.49 ,  "23", 0, roundHalfTowardsZero );
    ROUNDING_TEST(   23.499,  "23", 0, roundHalfTowardsZero );
    ROUNDING_TEST(   23.5  ,  "23", 0, roundHalfTowardsZero );
    ROUNDING_TEST(   23.51 ,  "24", 0, roundHalfTowardsZero );
    ROUNDING_TEST(   23.511,  "24", 0, roundHalfTowardsZero );
    ROUNDING_TEST(   23.9  ,  "24", 0, roundHalfTowardsZero );
    ROUNDING_TEST(   23.99 ,  "24", 0, roundHalfTowardsZero );
    ROUNDING_TEST(   24.0  ,  "24", 0, roundHalfTowardsZero );
    ROUNDING_TEST(   24.1  ,  "24", 0, roundHalfTowardsZero );

    cout << endl;
    ROUNDING_TEST(  -23.0  , "-23", 0, roundHalfTowardsZero );
    ROUNDING_TEST(  -23.1  , "-23", 0, roundHalfTowardsZero );
    ROUNDING_TEST(  -23.49 , "-23", 0, roundHalfTowardsZero );
    ROUNDING_TEST(  -23.499, "-23", 0, roundHalfTowardsZero );
    ROUNDING_TEST(  -23.5  , "-23", 0, roundHalfTowardsZero );
    ROUNDING_TEST(  -23.51 , "-24", 0, roundHalfTowardsZero );
    ROUNDING_TEST(  -23.501, "-24", 0, roundHalfTowardsZero );
    ROUNDING_TEST(  -23.9  , "-24", 0, roundHalfTowardsZero );
    ROUNDING_TEST(  -23.99 , "-24", 0, roundHalfTowardsZero );
    ROUNDING_TEST(  -24.0  , "-24", 0, roundHalfTowardsZero );
    ROUNDING_TEST(  -24.1  , "-24", 0, roundHalfTowardsZero );

    // roundMath
    cout << endl;
    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.0  ,  "23", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(   23.1  ,  "23", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(   23.49 ,  "23", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(   23.499,  "23", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(   23.5  ,  "24", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(   23.51 ,  "24", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(   23.501,  "24", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(   23.9  ,  "24", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(   23.99 ,  "24", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(   24.0  ,  "24", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(   24.1  ,  "24", 0, roundHalfTowardsInf  );

    cout << endl;
    ROUNDING_TEST(  -23.0  , "-23", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(  -23.1  , "-23", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(  -23.49 , "-23", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(  -23.499, "-23", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(  -23.5  , "-24", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(  -23.51 , "-24", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(  -23.501, "-24", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(  -23.9  , "-24", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(  -23.99 , "-24", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(  -24.0  , "-24", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(  -24.1  , "-24", 0, roundHalfTowardsInf  );

    cout << endl;
    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.0  ,  "23", 0, roundHalfToEven      );
    ROUNDING_TEST(   23.1  ,  "23", 0, roundHalfToEven      );
    ROUNDING_TEST(   23.49 ,  "23", 0, roundHalfToEven      );
    ROUNDING_TEST(   23.499,  "23", 0, roundHalfToEven      );
    ROUNDING_TEST(   23.5  ,  "24", 0, roundHalfToEven      );
    ROUNDING_TEST(   23.51 ,  "24", 0, roundHalfToEven      );
    ROUNDING_TEST(   23.501,  "24", 0, roundHalfToEven      );
    ROUNDING_TEST(   23.9  ,  "24", 0, roundHalfToEven      );
    ROUNDING_TEST(   23.99 ,  "24", 0, roundHalfToEven      );

    cout << endl;
    ROUNDING_TEST(   24.0  ,  "24", 0, roundHalfToEven      );
    ROUNDING_TEST(   24.1  ,  "24", 0, roundHalfToEven      );
    ROUNDING_TEST(   24.49 ,  "24", 0, roundHalfToEven      );
    ROUNDING_TEST(   24.499,  "24", 0, roundHalfToEven      );
    ROUNDING_TEST(   24.5  ,  "24", 0, roundHalfToEven      );
    ROUNDING_TEST(   24.51 ,  "25", 0, roundHalfToEven      );
    ROUNDING_TEST(   24.501,  "25", 0, roundHalfToEven      );
    ROUNDING_TEST(   24.9  ,  "25", 0, roundHalfToEven      );
    ROUNDING_TEST(   24.99 ,  "25", 0, roundHalfToEven      );

    cout << endl;
    ROUNDING_TEST(  -23.0  , "-23", 0, roundHalfToEven      );
    ROUNDING_TEST(  -23.1  , "-23", 0, roundHalfToEven      );
    ROUNDING_TEST(  -23.49 , "-23", 0, roundHalfToEven      );
    ROUNDING_TEST(  -23.499, "-23", 0, roundHalfToEven      );
    ROUNDING_TEST(  -23.5  , "-24", 0, roundHalfToEven      );
    ROUNDING_TEST(  -23.51 , "-24", 0, roundHalfToEven      );
    ROUNDING_TEST(  -23.501, "-24", 0, roundHalfToEven      );
    ROUNDING_TEST(  -23.9  , "-24", 0, roundHalfToEven      );
    ROUNDING_TEST(  -23.99 , "-24", 0, roundHalfToEven      );

    cout << endl;
    ROUNDING_TEST(  -24.0  , "-24", 0, roundHalfToEven      );
    ROUNDING_TEST(  -24.1  , "-24", 0, roundHalfToEven      );
    ROUNDING_TEST(  -24.49 , "-24", 0, roundHalfToEven      );
    ROUNDING_TEST(  -24.499, "-24", 0, roundHalfToEven      );
    ROUNDING_TEST(  -24.5  , "-24", 0, roundHalfToEven      );
    ROUNDING_TEST(  -24.51 , "-25", 0, roundHalfToEven      );
    ROUNDING_TEST(  -24.501, "-25", 0, roundHalfToEven      );
    ROUNDING_TEST(  -24.9  , "-25", 0, roundHalfToEven      );
    ROUNDING_TEST(  -24.99 , "-25", 0, roundHalfToEven      );

    /*
    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.5,  "23", 0, roundHalfToOdd       );
    ROUNDING_TEST(   22.5,  "23", 0, roundHalfToOdd       );
    ROUNDING_TEST(  -23.5, "-23", 0, roundHalfToOdd       );
    ROUNDING_TEST(  -22.5, "-23", 0, roundHalfToOdd       );
    */

    cout << endl;
    cout << "------------------------------" << endl;
    cout << endl;

    cout << "Failed " << totalRoundingTestsFailed << " rounding tests from total " << totalRoundingTests << endl;
    if (!totalRoundingTestsFailed)
        cout << "+++ All rounding tests passed"  << endl;

    return 0;
}



