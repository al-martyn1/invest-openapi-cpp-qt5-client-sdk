#pragma once

/*

    Измерение пропускной способности (чего-либо)


    У каждого события (например, приход сетевого пакета) есть метка времени (которую мы присваиваем сами),
    и полезная нагрузка (например, размер пакета).

    Мы задаём интервал, на котором хотим измерить пропускную способность или просто 
    подсчитать количество событий.



*/


#include "iterable_queue.h"




//----------------------------------------------------------------------------
//namespace invest_openapi
//{




template< typename TimeStampType, typename PayloadValueType >
struct BandwidthMeterQlement
{
    TimeStampType             timeStamp;
    PayloadValueType          payload  ;

}; // BandwidthMeterQlement




//----------------------------------------------------------------------------
template< typename TimeStampType
        , typename TimeStampMonotonicGetterType
        , typename PayloadValueType
        , typename IterableQueueType = iterable_queue< BandwidthMeterQlement< TimeStampType, PayloadValueType > >
        >
class BandwidthMeter
{
    
public:



    typedef typename TimeStampType                         timestamp_type         ;
    typedef typename TimeStampMonotonicGetterType          timestamp_getter_type  ;
    typedef typename PayloadValueType                      payload_type           ;

    typedef typename IterableQueueType                     queue_type             ;
    typedef typename queue_type                            container_type         ;
    typedef typename container_type::size_type             size_type              ;
    typedef typename container_type::value_type            value_type             ;


protected:


    timestamp_getter_type          m_timestampGetter;
    container_type                 m_queue;

    timestamp_type                 m_interval;


public:


    BandwidthMeter()
    : m_timestampGetter()
    , m_queue()
    {}

    BandwidthMeter( const timestamp_getter_type &tsGetter)
    : m_timestampGetter(tsGetter)
    , m_queue()
    {}


    void setInterval( timestamp_type interval )
    {
        m_interval = interval;
    }

    timestamp_type interval( ) const
    {
        return m_interval;
    }

    timestamp_getter_type getter() const
    {
        return m_timestampGetter;
    }

    timestamp_type timestamp() const
    {
        return m_timestampGetter();
    }


    void push( const payload_type &pl = payload_type() )
    {
        m_queue.push( value_type{ m_timestampGetter(), pl } );
    }


    // Возвращаем количество событий, входящих в заданный интервал от текущего момента назад
    // Старые события удаляются из очереди
    size_type size()
    {
        if (m_queue.empty()) return 0;

        timestamp_type stampNow  = m_timestampGetter();
        timestamp_type ageLim    = stampNow - m_interval;

        auto stampOldest = m_queue.front().timeStamp;

        while(!m_queue.empty() && stampOldest<ageLim)
        {
            m_queue.pop();
            stampOldest = m_queue.front().timeStamp;
        }

        return m_queue.size();

    }
    
    template< typename TAccumulator >
    TAccumulator calcPayload( TAccumulator acc )
    {
        size();

        typename container_type::const_iterator it = m_queue.begin();
        for(; it != m_queue.end(); ++it )
        {
            acc(it->payload);
        }

        return acc;
    }



}; // class BandwidthMeter







//----------------------------------------------------------------------------




//----------------------------------------------------------------------------

//} // namespace invest_openapi
