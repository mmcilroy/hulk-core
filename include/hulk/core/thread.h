
#ifndef _hulk_core_thread_h_
#define _hulk_core_thread_h_

#include <stdio.h>
#include <time.h>
#include <pthread.h>

namespace hulk {
namespace core {

// -----------------------------------------------------------------------------
void sleep_ms( int ms );

class thread
{
public:
    void start() {
        pthread_create( &_handle, NULL, &thread::pthread_fn, (void*)this );
    }

    void join() {
        pthread_join( _handle, NULL );
    }

    static void yield() {
        pthread_yield();
    }

private:
    virtual void run() = 0;

    static void* pthread_fn( void* t ) {
        ((thread*)t)->run();
    }

    pthread_t _handle;
};

// -----------------------------------------------------------------------------
class mutex
{
public:
    mutex() {
        pthread_mutex_init( &_mutex, NULL );
    }

    void lock() {
        pthread_mutex_lock( &_mutex );
    }

    void unlock() {
        pthread_mutex_unlock( &_mutex );
    }

private:
    pthread_mutex_t _mutex;
};

// -----------------------------------------------------------------------------
class lock_guard
{
public:
    lock_guard( mutex& mutex ) : _mutex( mutex ) { 
        _mutex.lock();
    }

    ~lock_guard() {
        _mutex.unlock();
    }

private:
    mutex _mutex;
};

}
}

#endif
