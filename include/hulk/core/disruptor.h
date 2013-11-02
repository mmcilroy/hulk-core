
#ifndef _hulk_disruptor_h_
#define _hulk_disruptor_h_

#include "hulk/core/thread.h"
#include <cstdlib>

namespace hulk {

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
class reader
{
template< class U > friend class writer;
public:
    reader( ring_buffer<T>& rb );
    reader( reader<T>& r );

    inline const T& next();
    inline int available();

//private:
    ring_buffer<T>& _rb;
    sequence& _barrier;
    sequence _seq;
};

// -----------------------------------------------------------------------------
template< class T >
class writer
{
public:
    writer( reader<T>& r );

    inline T& next();
    inline void commit();

private:
    ring_buffer<T>& _rb;
    sequence& _barrier;
};

// -----------------------------------------------------------------------------
template< class T >
class reader_thread : public thread
{
public:
    reader_thread( reader<T>& r );
    reader_thread( ring_buffer<T>& rb );
    reader_thread( reader_thread<T>& rt );

    inline reader<T>& get_reader();

protected:
    virtual void process( const T& item ) = 0;

private:
    virtual void run();

    reader<T> _reader;
};

// -----------------------------------------------------------------------------
sequence::sequence()
: _val( 0 )
{
}

sequence::value sequence::add( size_t s )
{
    __sync_fetch_and_add( &_val, s );
}

sequence::value sequence::get() const
{
    return _val;
}

// -----------------------------------------------------------------------------
template< class T >
ring_buffer<T>::ring_buffer( size_t s )
: _buffer( new T[s] ),
  _size( s )
{
}

template< class T >
ring_buffer<T>::~ring_buffer()
{
    if( _buffer ) {
        delete [] _buffer;
    }
}

template< class T >
T& ring_buffer<T>::next()
{
    return at( _sequence );
}

template< class T >
void ring_buffer<T>::commit()
{
    _sequence.add();
}

template< class T >
size_t ring_buffer<T>::size() const
{
    return _size;    
}

template< class T >
T& ring_buffer<T>::at( const sequence& s ) const
{
    return _buffer[ s.get() % _size ];
}

template< class T >
sequence& ring_buffer<T>::get_sequence()
{
    return _sequence;
}

// -----------------------------------------------------------------------------
template< class T >
reader< T >::reader( ring_buffer<T>& rb )
: _rb( rb ),
  _barrier( rb.get_sequence() )
{
}

template< class T >
reader< T >::reader( reader<T>& r )
: _rb( r._rb ),
  _barrier( r._seq )
{
}

template< class T >
const T& reader< T >::next()
{
    while( _barrier.get() - _seq.get() <= 0 ) {
        thread::yield();
    }

    const T& e = _rb.at( _seq );
    _seq.add();
    return e;
}

template< class T >
int reader< T >::available()
{
    return _barrier.get() - _seq.get();
}

// -----------------------------------------------------------------------------
template< class T >
writer< T >::writer( reader< T >& r )
: _rb( r._rb ),
  _barrier( r._seq )
{
}

template< class T >
T& writer< T >::next()
{
    while( _rb.get_sequence().get() - _barrier.get() >= _rb.size() ) {
        thread::yield();
    }

    return _rb.next();
}

template< class T >
void writer< T >::commit()
{
    _rb.commit();
}

// -----------------------------------------------------------------------------
template< class T >
reader_thread< T >::reader_thread( reader< T >& r )
: _reader( r )
{
}

template< class T >
reader_thread< T >::reader_thread( ring_buffer< T >& rb )
: _reader( rb )
{
}

template< class T >
reader_thread< T >::reader_thread( reader_thread< T >& rt )
: _reader( rt._reader )
{
}

template< class T >
void reader_thread< T >::run()
{
    while( 1 ) {
        process( _reader.next() );
    }
}

template< class T >
reader< T >& reader_thread< T >::get_reader()
{
    return _reader;
}

}

#endif
