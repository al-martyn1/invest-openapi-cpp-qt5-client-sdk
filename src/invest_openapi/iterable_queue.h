#pragma once


// Без пространства имён.
// Явно это не часть invest_openapi, перенесу в какой-нибудь свой набор велосипедов потом

//----------------------------------------------------------------------------


#include <memory>
#include <vector>
#include <utility>
#include <sstream>
#include <string>

//----------------------------------------------------------------------------






//----------------------------------------------------------------------------
template< class T
        , class Allocator = std::allocator<T>
        >
class iterable_queue
{

    //------------------------------

public:


    typedef std::vector< T, Allocator >                         container_type            ;


    typedef  typename container_type::value_type                value_type                ;
                                                                                
    typedef  typename container_type::allocator_type            allocator_type            ;
                                                                                
    typedef  typename container_type::size_type                 size_type                 ;
    typedef  typename container_type::difference_type           difference_type           ;
                                                                                
    typedef  typename container_type::reference                 reference                 ;
    typedef  typename container_type::const_reference           const_reference           ;
                                                                                
    typedef  typename container_type::pointer                   pointer                   ;
    typedef  typename container_type::const_pointer             const_pointer             ;
                                                                                
    typedef  typename container_type::iterator                  iterator                  ;
    typedef  typename container_type::const_iterator            const_iterator            ;
    typedef  typename container_type::reverse_iterator          reverse_iterator          ;
    typedef  typename container_type::const_reverse_iterator    const_reverse_iterator    ;


    //------------------------------

protected:


    size_type            m_firstElementIndex;
    container_type       m_container;


    //------------------------------

public:


    //------------------------------
    // Ctors

    iterable_queue() 
    : m_firstElementIndex(0), m_container()
    {}


    iterable_queue( const iterable_queue& other )
    : m_firstElementIndex(0)
    , m_container( other.begin()+other.m_firstElementIndex, other.end() )
    {}


    //------------------------------
    // Common methods

    void swap( iterable_queue& other )
    {
        std::swap( m_firstElementIndex, other.m_firstElementIndex );
        m_container.swap( other.m_container );
    }


    iterable_queue& operator=( const iterable_queue &other )
    {
        if (this==&other)
            return *this;

        iterable_queue tmp(other);
        swap(tmp);

        return *this;
    }


    //------------------------------

    void shrink_to_fit()
    {
        iterable_queue tmp(*this);
        swap(tmp);
    }

    size_type size() const
    {
        return m_container.size() - m_firstElementIndex;
    }

    bool empty() const
    {
        return size()==0;
    }

    // Most common, but not exist in queue
    // А нужно ли?
    /*
    size_type capacity() const
    {
        m_container.capacity() + m_firstElementIndex;
    }
    */

    //------------------------------
    // Queue methods

    reference front()
    {
        return m_container[m_firstElementIndex];
    }

    const_reference front() const
    {
        return m_container[m_firstElementIndex];
    }


    reference back()
    {
        return m_container.back();
    }

    const_reference back() const
    {
        return m_container.back();
    }


    void push( const value_type& value )
    {
        m_container.push_back(value);
    }

    void push( value_type&& value )
    {
        m_container.push_back(value);
    }


    template< typename... Args >
    void emplace( Args&&... args )
    {
        m_container.emplace_back( std::forward<Args>(args)... );
    }


protected:

    // Чем жирнее тип, тем чаще его двигаем - чтобы избежать редких крупных просадок производительности, заменяем их на почаще и поменьше
    constexpr size_type getMoveLimit()
    {
        return sizeof(value_type) <= 8   // POD type or struct
               ? 8192u
               : sizeof(value_type) <= 32  // POD struct
                 ? 2048
                 : 256
               ;
    }

public:


    void pop()
    {
        // На кого перекладывать ответственность, если что-то пошло не так?
        // Пытаться самому разрулить или пусть его, падает?
        // Наверное, пусть падает обычным способом

        ++m_firstElementIndex;

        #if defined(_DEBUG) || defined(DEBUG)

            const size_type moveLimit = 2;

        #else

            const size_type moveLimit = getMoveLimit();

        #endif

        if ( m_firstElementIndex>moveLimit // на мелочи нет смысла дёргаться
          && m_firstElementIndex>m_container.size()/2 
           )
        {
            m_container.erase( m_container.begin(), m_container.begin()+m_firstElementIndex );
            m_firstElementIndex = 0;
        }
    }


    //------------------------------
    // Iterable ипостася

    iterator begin()
    {
        return m_container.begin()+m_firstElementIndex;
    }

    const_iterator begin() const
    {
        return m_container.begin()+m_firstElementIndex;
    }


    iterator end()
    {
        return m_container.end();
    }

    const_iterator end() const
    {
        return m_container.end();
    }

    // Остальное добью при необходимости




}; // class iterable_queue

//----------------------------------------------------------------------------






//----------------------------------------------------------------------------
// https://en.cppreference.com/w/cpp/string/basic_string/to_string
// See also https://en.cppreference.com/w/cpp/utility/to_chars
template< class T, class Allocator > inline
std::string to_string( const iterable_queue<T, Allocator> &q )
{
    std::ostringstream oss;

    bool bFirst = true;

    for( const auto e : q )
    {
        if (!bFirst)
           oss << ", ";
        else
           bFirst = false;

        oss << "{" << e << "}";
    }


    return oss.str();
}


