
#ifndef _hulk_shared_ptr_h_
#define _hulk_shared_ptr_h_

#include <stdexcept>

namespace hulk {

template< class T >
class shared_ptr
{
public:
    shared_ptr()
    : _rc( 0 ),
      _pointee( 0 )
    {
    }

    shared_ptr( T* o )
    : _pointee( o )
    {
        _rc = new unsigned int;
        *_rc = 1;
    }

    shared_ptr( const shared_ptr<T>& sp )
    {
        assign( sp );
    }

    ~shared_ptr()
    {
        reset();
    }

    T* get() const
    {
        return _pointee;
    }

    void reset()
    {
        if( _pointee && _rc && --(*_rc) == 0 )
        {
            delete _pointee;
            delete _rc;
        }

        _pointee = 0;
        _rc = 0;
    }

    shared_ptr<T>& operator=( const shared_ptr<T>& rhs )
    {
        return assign( rhs );
    }

    T* operator->() const
    {
        if( !_pointee ) {
            throw std::runtime_error( "null shared_ptr" );
        }

        return _pointee;
    }

    T& operator*() const
    {
        if( !_pointee ) {
            throw std::runtime_error( "null shared_ptr" );
        }

        return *_pointee;
    }

    operator bool() const
    {
        return _pointee != 0;
    }

private:
    shared_ptr<T>& assign( const shared_ptr<T>& sp )
    {
        _pointee = sp._pointee;
        _rc = sp._rc;

        if( _rc ) {
            ++(*_rc);
        }

        return *this;
    }

    T* _pointee;
    unsigned int* _rc;
};

}

#endif
