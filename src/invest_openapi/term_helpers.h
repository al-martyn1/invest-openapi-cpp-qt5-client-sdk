#pragma once

#include <iostream>
#include <sstream>
#include <iomanip>


#include "../cpp/cpp.h"
#include "marty_decimal.h"


namespace invest_openapi
{


// Alignment left    - <0
//           right   - >0
//           center  - ==0


//----------------------------------------------------------------------------
inline
std::ostream& termClearScreen( std::ostream &os, unsigned numLines = 50 )
{
    for( unsigned i = 0; i!=numLines; ++i)
    {
        os << std::endl;
    }

    return os;
}

//----------------------------------------------------------------------------




} // namespace invest_openapi

//----------------------------------------------------------------------------
