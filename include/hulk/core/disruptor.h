
#ifndef _hulk_core_disruptor_h_
#define _hulk_core_disruptor_h_

#include <cstdlib>

namespace hulk {
namespace core {

class sequence
{
public:
    typedef unsigned long long value;

    sequence() : _val( 0 ) {
        ;
    }

    inline value add( size_t s = 1 ) {
        _val += s; // __sync_fetch_and_add( &_val, s );
    }

    inline value get() const {
        return _val;
    }

private:
    value _val;    
};

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

}
}

#endif
