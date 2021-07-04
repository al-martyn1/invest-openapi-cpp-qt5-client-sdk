#pragma once

#include <iostream>
#include <sstream>
#include <iomanip>
#include <map>
#include <exception>
#include <stdexcept>

#include <QString>
#include <QStringList>


#include "../cpp/cpp.h"
#include "marty_decimal.h"


#if defined(UMBA_SIMPLE_FORMATTER_H)

    #include "umba/char_writers.h"

    #define INVEST_OPENAPI_TEXT_TERMINAL_ENABLED_COLORS

#endif



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






//----------------------------------------------------------------------------
inline
const std::map< QString, unsigned >& getTermColorsMap()
{
    static std::map< QString, unsigned > colors;

    if (colors.empty())
    {
        #if defined(INVEST_OPENAPI_TEXT_TERMINAL_ENABLED_COLORS)
        colors["BLACK"]   = umba::term::colors::black    ;
        colors["RED"]     = umba::term::colors::red      ;
        colors["GREEN"]   = umba::term::colors::green    ;
        colors["YELLOW"]  = umba::term::colors::yellow   ;
        colors["BLUE"]    = umba::term::colors::blue     ;
        colors["MAGENTA"] = umba::term::colors::magenta  ;
        colors["CYAN"]    = umba::term::colors::cyan     ;
        colors["WHITE"]   = umba::term::colors::white    ;

        colors["BRIGHT"]  = umba::term::colors::bright   ;
        colors["BLINK"]   = umba::term::colors::blink    ;
        colors["INVERT"]  = umba::term::colors::invert   ;
        #endif
    }

    return colors;

}

//----------------------------------------------------------------------------
inline
unsigned termColorFromQStringList( const QStringList &l )
{
    unsigned resColor = 0;

    const std::map< QString, unsigned >& colors = getTermColorsMap();

    for( const auto &clr : l )
    {
        auto clrUpper = clr.toUpper();

        bool bg = false;

        if ( clrUpper.startsWith("BG_", Qt::CaseInsensitive) )
        {
            bg = true;
            clrUpper.remove(0,3);
        }
        if ( clrUpper.startsWith("BG-", Qt::CaseInsensitive) )
        {
            bg = true;
            clrUpper.remove(0,3);
        }
        else if ( clrUpper.startsWith("BG", Qt::CaseInsensitive) )
        {
            bg = true;
            clrUpper.remove(0,2);
        }

        std::map< QString, unsigned >::const_iterator cit = colors.find(clrUpper);
        if (cit == colors.end())
            throw std::runtime_error( std::string("invest_openapi::termColorFromQStringList: Invalid color value: '") + clr.toStdString() + std::string("'"));


        unsigned clrFound = cit->second;

        #if defined(INVEST_OPENAPI_TEXT_TERMINAL_ENABLED_COLORS)
            if ( bg && ( clrFound==umba::term::colors::bright
                      || clrFound==umba::term::colors::blink 
                      || clrFound==umba::term::colors::invert
                       )
           
               )
               throw std::runtime_error( std::string("invest_openapi::termColorFromQStringList: Color attribute '") + clr.toStdString() + std::string("' can't be set for background") );
        #endif

        if (bg)   
            resColor |= clrFound<<8;
        else
            resColor |= clrFound;
           
    }

    return resColor;

}


// umba::term::colors::





} // namespace invest_openapi

//----------------------------------------------------------------------------
