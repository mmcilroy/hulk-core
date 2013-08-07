
#ifndef _hulk_core_thread_h_
#define _hulk_core_thread_h_

#include "pthread.h"

namespace hulk {
namespace core {

class thread
{
public:
    thread() {
        pthread_create( &_handle, NULL, &thread::pthread_fn, (void*)this );
    }

    void join() {
        pthread_join( _handle, NULL );
    }

    void yield() {
        pthread_yield();
    }

private:
    virtual void run() = 0;

    static void* pthread_fn( void* t ) {
        ((thread*)t)->run();
    }

    pthread_t _handle;
};

}
}

#endif
