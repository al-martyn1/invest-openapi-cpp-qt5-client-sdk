#pragma once

#include <QElapsedTimer>


struct DefaultMonotonicTimestamper
{

    typedef qint64    timestamp_type;

    DefaultMonotonicTimestamper() 
    : m_pTimer( QSharedPointer<QElapsedTimer>( new QElapsedTimer() ) )
    {
        m_pTimer->start();
    }

    DefaultMonotonicTimestamper( const DefaultMonotonicTimestamper &other ) 
    : m_pTimer(other.m_pTimer)
    {}

    timestamp_type operator()() const
    {
        return m_pTimer->elapsed();
    }


protected:

    QSharedPointer<QElapsedTimer>   m_pTimer;


};