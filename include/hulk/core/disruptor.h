
#ifndef _hulk_core_disruptor_h_
#define _hulk_core_disruptor_h_

#include "hulk/core/thread.h"
#include <cstdlib>

namespace hulk {
namespace core {

// -----------------------------------------------------------------------------
class sequence
{
public:
    typedef unsigned long long value;

    inline sequence();
    inline value add( size_t s = 1 );
    inline value get() const;

private:
    value _val;
};

// -----------------------------------------------------------------------------
template< class T >
class ring_buffer
{
public:
    ring_buffer( size_t s );
    ~ring_buffer();

    inline T& next();
    inline T& at( const sequence& s ) const;
    inline void commit();
    inline size_t size() const;
    inline sequence& get_sequence();

private:
  T* _buffer;
  sequence _sequence;
  size_t _size;
};

// -----------------------------------------------------------------------------
template< class T >
class reader_thread : public thread
{
public:
    inline reader_thread( ring_buffer<T>& rb );
    inline reader_thread( reader_thread<T>& rt );

    inline sequence& get_sequence();
    inline ring_buffer<T>& get_ring_buffer();

protected:
    virtual void process( const T& item ) = 0;

private:
    virtual void run();

    ring_buffer<T>& _rb;
    sequence& _barrier;
    sequence _seq;
};

// -----------------------------------------------------------------------------
sequence::sequence() : _val( 0 )
{
    ;
}

sequence::value sequence::add( size_t s )
{
    _val += s; // __sync_fetch_and_add( &_val, s );
}

sequence::value sequence::get() const
{
    return _val;
}

// -----------------------------------------------------------------------------
template< class T > ring_buffer<T>::ring_buffer( size_t s )
: _buffer( new T[s] ),
  _size( s )
{
}

template< class T > ring_buffer<T>::~ring_buffer()
{
    if( _buffer ) {
        delete [] _buffer;
    }
}

template< class T > T& ring_buffer<T>::next()
{
    return at( _sequence );
}

template< class T > void ring_buffer<T>::commit()
{
    _sequence.add();
}

template< class T > size_t ring_buffer<T>::size() const
{
    return _size;    
}

template< class T > T& ring_buffer<T>::at( const sequence& s ) const
{
    return _buffer[ s.get() % _size ];
}

template< class T > sequence& ring_buffer<T>::get_sequence()
{
    return _sequence;
}

// -----------------------------------------------------------------------------
template< class T >
reader_thread< T >::reader_thread( ring_buffer< T >& rb )
: _rb( rb ),
  _barrier( rb.get_sequence() )
{
}

template< class T >
reader_thread< T >::reader_thread( reader_thread< T >& rt )
: _rb( rt._rb ),
  _barrier( rt._seq )
{
}

template< class T >
sequence& reader_thread< T >::get_sequence()
{
    return _seq;
}

template< class T >
ring_buffer< T >& reader_thread< T >::get_ring_buffer()
{
    return _rb;
}

template< class T >
void reader_thread< T >::run()
{
    while( 1 )
    {
        if( _barrier.get() - _seq.get() ) {
            process( _rb.at( _seq ) ); _seq.add();
        } else {
            yield();
        }
    }
}

}
}

#endif
