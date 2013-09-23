
#ifndef _hulk_core_shared_ptr_h_
#define _hulk_core_shared_ptr_h_

#include <iostream>

namespace hulk {
namespace core {

template< class T >
class shared_ptr
{
public:
    shared_ptr( T* o )
    {
        _pointee = o;
        _rc = new unsigned int;
        *_rc = 1;
    }

    shared_ptr( shared_ptr<T>& sp )
    {
        _pointee = sp._pointee;
        _rc = sp._rc;
        ++(*_rc);
    }

    ~shared_ptr()
    {
        if( --(*_rc) == 0 )
        {
            delete _pointee;
            delete _rc;
        }
    }

    T* get() const
    {
        return _pointee;
    }

    T* operator->() const
    {
        return _pointee;
    }

    T& operator*() const
    {
        return *_pointee;
    }

private:
    T* _pointee;
    unsigned int* _rc;
};

}
}

#endif
