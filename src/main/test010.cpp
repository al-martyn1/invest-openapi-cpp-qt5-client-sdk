/*! \file
    \brief Decimal type test

 */

#include <iostream>
#include <exception>
#include <stdexcept>
#include <sstream>

#include <QCoreApplication>
#include <QString>
#include <QSettings>
#include <QTest>
#include <QDir>
#include <QElapsedTimer>

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
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("test010");
    QCoreApplication::setApplicationVersion("1.0");

    QCoreApplication::setOrganizationName("al-martyn1");
    QCoreApplication::setOrganizationDomain("https://github.com/al-martyn1/");

    using std::cout;
    using std::endl;

    using marty::Decimal;
    using marty::DecimalPrecision;
    using marty::toString;
    using marty::fromString;

    #define PRINT( var ) cout << #var << ": "<< var << endl;

    Decimal d1_u64_3   = (std::uint64_t)(3)  ; PRINT(d1_u64_3);
    Decimal d1_i64_7   = (std::int64_t )(7)  ; PRINT(d1_i64_7);
    Decimal d1_i64_m10 = (std::int64_t )(-10); PRINT(d1_i64_m10);
    Decimal d1_i64_15_000 = Decimal(15, DecimalPrecision(3)); PRINT(d1_i64_15_000);
    Decimal d1 = fromString("123.010");        PRINT(d1);
    Decimal d2 = fromString("13.10");          PRINT(d2);
    Decimal d1_2_sum = d1 + d2;                PRINT(d1_2_sum);  // 123.010 + 13.10 =  136.11
    Decimal d1_2_raz = d1 - d2;                PRINT(d1_2_raz);  // 123.010 - 13.10 =  109.91
    Decimal d1_2_mul = d1 * d2;                PRINT(d1_2_mul);  // 123.010 * 13.10 = 1611.431
    Decimal d1_2_div = d1 / d2;                PRINT(d1_2_div);  // 123.010 / 13.10 =    9.390076335877862595419847328
    Decimal d1_2_ost = d1 % d2;                PRINT(d1_2_ost);  // 123.010 % 13.10 =    5.11

    auto    d1_2_sum_plus_15_a = d1_2_sum + (std::int64_t )(15); PRINT(d1_2_sum_plus_15_a);
    auto    d1_2_sum_plus_15_b = d1_2_sum + (std::uint64_t)(15); PRINT(d1_2_sum_plus_15_b);
    auto    d1_2_sum_plus_15_c = d1_2_sum + (int          )(15); PRINT(d1_2_sum_plus_15_c);
    auto    d1_2_sum_plus_15_d = d1_2_sum + (unsigned     )(15); PRINT(d1_2_sum_plus_15_d);
    auto    d1_2_sum_plus_15_e = d1_2_sum + (double       )(15); PRINT(d1_2_sum_plus_15_e);


    Decimal divideD1   = Decimal( 100, DecimalPrecision(2) );
    Decimal divideD2   = Decimal(  25, DecimalPrecision(1) );
    Decimal divideRes  = divideD1.divide(divideD2, 6);
    
    cout << divideRes.toString() << endl;

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
                    bool bGood /* Johny */ = (strRes==strResForCompare);                 \
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

    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.0,  "23", 0, roundFloor           );
    ROUNDING_TEST(   23.7,  "23", 0, roundFloor           );
    ROUNDING_TEST(   24.0,  "24", 0, roundFloor           );

    cout << endl;
    ROUNDING_TEST(  -23.0, "-23", 0, roundFloor           );
    ROUNDING_TEST(  -23.2, "-24", 0, roundFloor           );
    ROUNDING_TEST(  -24.0, "-24", 0, roundFloor           );

    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.0,  "23", 0, roundCeil            );
    ROUNDING_TEST(   23.2,  "24", 0, roundCeil            );
    ROUNDING_TEST(   24.0,  "24", 0, roundCeil            );

    cout << endl;
    ROUNDING_TEST(  -23.0, "-23", 0, roundCeil            );
    ROUNDING_TEST(  -23.7, "-23", 0, roundCeil            );
    ROUNDING_TEST(  -24.0, "-24", 0, roundCeil            );

    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.0,  "23", 0, roundTrunc           ); // roundTowardsZero
    ROUNDING_TEST(   23.7,  "23", 0, roundTrunc           ); // roundTowardsZero
    ROUNDING_TEST(   24.0,  "24", 0, roundTrunc           ); // roundTowardsZero

    cout << endl;
    ROUNDING_TEST(  -23.0, "-23", 0, roundTrunc           ); // roundTowardsZero
    ROUNDING_TEST(  -23.7, "-23", 0, roundTrunc           ); // roundTowardsZero
    ROUNDING_TEST(  -24.0, "-24", 0, roundTrunc           ); // roundTowardsZero

    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.0,  "23", 0, roundTowardsInf      ); // roundAwayFromZero
    ROUNDING_TEST(   23.2,  "24", 0, roundTowardsInf      ); // roundAwayFromZero
    ROUNDING_TEST(   24.0,  "24", 0, roundTowardsInf      ); // roundAwayFromZero

    cout << endl;
    ROUNDING_TEST(  -23.0, "-23", 0, roundTowardsInf      ); // roundAwayFromZero
    ROUNDING_TEST(  -23.2, "-24", 0, roundTowardsInf      ); // roundAwayFromZero
    ROUNDING_TEST(  -24.0, "-24", 0, roundTowardsInf      ); // roundAwayFromZero

    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.1,  "23", 0, roundHalfUp          );
    ROUNDING_TEST(   23.5,  "24", 0, roundHalfUp          );
    ROUNDING_TEST(   23.9,  "24", 0, roundHalfUp          );

    cout << endl;
    ROUNDING_TEST(  -23.1, "-23", 0, roundHalfUp          );
    ROUNDING_TEST(  -23.5, "-23", 0, roundHalfUp          );
    ROUNDING_TEST(  -23.9, "-24", 0, roundHalfUp          );

    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.1,  "23", 0, roundHalfDown        );
    ROUNDING_TEST(   23.5,  "23", 0, roundHalfDown        );
    ROUNDING_TEST(   23.9,  "24", 0, roundHalfDown        );
    cout << endl;
    ROUNDING_TEST(  -23.1, "-23", 0, roundHalfDown        );
    ROUNDING_TEST(  -23.5, "-24", 0, roundHalfDown        );
    ROUNDING_TEST(  -23.9, "-24", 0, roundHalfDown        );

    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.1,  "23", 0, roundHalfTowardsZero );
    ROUNDING_TEST(   23.5,  "23", 0, roundHalfTowardsZero );
    ROUNDING_TEST(   23.9,  "24", 0, roundHalfTowardsZero );

    cout << endl;
    ROUNDING_TEST(  -23.1, "-23", 0, roundHalfTowardsZero );
    ROUNDING_TEST(  -23.5, "-23", 0, roundHalfTowardsZero );
    ROUNDING_TEST(  -23.9, "-24", 0, roundHalfTowardsZero );

    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.1,  "23", 0, roundHalfTowardsInf  ); // roundMath
    ROUNDING_TEST(   23.5,  "24", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(   23.9,  "24", 0, roundHalfTowardsInf  );

    cout << endl;
    ROUNDING_TEST(  -23.1, "-23", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(  -23.5, "-24", 0, roundHalfTowardsInf  );
    ROUNDING_TEST(  -23.9, "-24", 0, roundHalfTowardsInf  );

    cout << "------------------------------" << endl;
    ROUNDING_TEST(   23.1,  "23", 0, roundHalfToEven      );
    ROUNDING_TEST(   23.5,  "24", 0, roundHalfToEven      );
    ROUNDING_TEST(   23.9,  "24", 0, roundHalfToEven      );

    cout << endl;
    ROUNDING_TEST(   24.1,  "24", 0, roundHalfToEven      );
    ROUNDING_TEST(   24.5,  "24", 0, roundHalfToEven      );
    ROUNDING_TEST(   24.9,  "25", 0, roundHalfToEven      );

    cout << endl;
    ROUNDING_TEST(  -23.1, "-23", 0, roundHalfToEven      );
    ROUNDING_TEST(  -23.5, "-24", 0, roundHalfToEven      );
    ROUNDING_TEST(  -23.9, "-24", 0, roundHalfToEven      );

    cout << endl;
    ROUNDING_TEST(  -24.1, "-24", 0, roundHalfToEven      );
    ROUNDING_TEST(  -24.5, "-24", 0, roundHalfToEven      );
    ROUNDING_TEST(  -24.9, "-25", 0, roundHalfToEven      );

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

    cout << "Failed " << totalRoundingTestsFailed << " tests from total " << totalRoundingTests << endl;
    if (!totalRoundingTestsFailed)
        cout << "+++ All rounding tests passed"  << endl;

    return 0;
}



