#pragma once


#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <utility>
#include <iterator>



//----------------------------------------------------------------------------
namespace invest_openapi {
namespace cpp_helpers {




//----------------------------------------------------------------------------
template< typename LineHandler > inline
void readLines( std::istream &in, LineHandler handler )
{
    std::string s;
    
    while( std::getline( in, s ) )
    {
        handler(s);
    }
}

//----------------------------------------------------------------------------
inline
std::vector<std::string> readLines( std::istream &in )
{
    std::string s;
    std::vector<std::string> res;

    auto handler = [&]( const std::string &str )
                      {
                          res.push_back(str);
                      };

    readLines( in, handler );

    return res;
}

//----------------------------------------------------------------------------
template< typename LineHandler > inline
void readLinesFromFile( const std::string &fname, LineHandler handler )
{
    std::ifstream in(fname.c_str());
    if (!in)
        return;

    readLines( in, handler );
}

//----------------------------------------------------------------------------
inline
std::vector<std::string> readLinesFromFile( const std::string &fname )
{
    std::ifstream in(fname.c_str());
    if (!in)
        return std::vector<std::string>();

    return readLines( in );
}

//----------------------------------------------------------------------------




//-----------------------------------------------------------------------------
//! Возвращает отрицание функционального объекта-предиката
/*! 
    \tparam ActualPred Тип предиката
    \tparam ParamType  Тип аргумента предиката
    \return Отрицание предиката
 */

template<typename ActualPred, typename ParamType>
struct not_pred
{
    const ActualPred &actualPred; //!< Ссылка на предикат для отрицания его результата
    //! Конструктор предиката отрицания
    not_pred( const ActualPred &a /*!< предикат для отрицания */ ) : actualPred(a) {}
    //! Оператор функционального объекта
    bool operator()( ParamType ch ) const { return !actualPred(ch); }
};

//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------
template<typename CharType>
struct space_pred
{
    bool operator()( CharType  ch ) const { return ch==(CharType)' '; /* is_space(ch); */  }
};

//-----------------------------------------------------------------------------
template<typename CharType>
struct space_or_tab_pred
{
    bool operator()( CharType ch )  const { return ch==(CharType)' ' || ch==(CharType)'\t';; /* is_space_ot_tab(ch); */  }
};

//-----------------------------------------------------------------------------
template<typename CharType>
struct space_or_tab_or_crlf_pred
{
    bool operator()( CharType ch )  const { return ch==(CharType)' ' || ch==(CharType)'\t' || ch==(CharType)'\r' || ch==(CharType)'\n';  }
};

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
template <typename StringType, typename TrimPred> inline void ltrim( StringType &s, const TrimPred &pred )
{
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), not_pred<TrimPred, typename StringType::value_type>(pred) ));
}

//-----------------------------------------------------------------------------
template <typename StringType, typename TrimPred > inline void rtrim(StringType &s, const TrimPred &pred)
{
    s.erase( std::find_if(s.rbegin(), s.rend(), not_pred<TrimPred,typename StringType::value_type>(pred) ).base(), s.end());
}

//-----------------------------------------------------------------------------
template <typename StringType, typename TrimPred> inline void trim(StringType &s, const TrimPred &pred)
{
    ltrim(s,pred); rtrim(s,pred);
}

//-----------------------------------------------------------------------------
template <typename StringType> inline void ltrim(StringType &s)
{
    ltrim(s, space_or_tab_pred<typename StringType::value_type>());
}

//-----------------------------------------------------------------------------
template <typename StringType> inline void rtrim(StringType &s)
{
    rtrim(s, space_or_tab_pred<typename StringType::value_type>());
}

//-----------------------------------------------------------------------------
template <typename StringType> inline void trim(StringType &s)
{
    trim(s, space_or_tab_pred<typename StringType::value_type>());
}

//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------
inline
bool splitToPair( std::string str, std::string &first, std::string &second, char ch, std::string::size_type pos = 0 )
{
    //std::string::size_type 
    pos = str.find( ch, pos );
    if (pos==std::string::npos)
    {
        first.assign(str, 0);
        second.clear();
        return false;
    }

    first.assign( str, 0, pos );
    second.assign( str, pos+1 );
    return true;
}

//-----------------------------------------------------------------------------
inline
void splitToVector( std::string str, std::vector<std::string> &vec, char ch )
{
    std::string f, s;
    while( splitToPair(str, f, s, ch) )
    {
        vec.push_back(f);
        str = s;
    }
    if (!str.empty())
       vec.push_back(f);
}

//----------------------------------------------------------------------------




//----------------------------------------------------------------------------
template< typename VectorType > inline
void trimStrings( VectorType &v, bool keepEmpty = false )
{
    typedef typename VectorType::value_type          vector_value_type;
    typedef typename vector_value_type::value_type   CharType;

    VectorType res; res.reserve( v.size() );

    for(auto s : v)
    {
        trim(s, space_or_tab_or_crlf_pred<CharType>());

        if (s.empty() && !keepEmpty)
            continue;

        res.push_back(s);
    }

    res.shrink_to_fit();

    v.swap(res);
}

//----------------------------------------------------------------------------

} // namespace cpp_helpers
} // namespace invest_openapi

