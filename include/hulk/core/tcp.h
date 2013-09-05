
#ifndef _hulk_core_tcp_
#define _hulk_core_tcp_

#include <sys/epoll.h>
#include <cstdlib>

namespace hulk {
namespace core {
namespace  tcp {

// -----------------------------------------------------------------------------
int bind( int port, int backlog=1024 );
int connect( const char* host, int port );
int non_blocking( int fd );

// -----------------------------------------------------------------------------
struct callback
{
    virtual void on_open( int fd ) {}
    virtual void on_close( int fd ) {}
    virtual void on_recv( int fd, const char* data, size_t len ) {}
};

// -----------------------------------------------------------------------------
class event_loop
{
public:
    event_loop( int max_events=1024 );
    void watch( int fd, bool listening, callback& cb );
    void dont_watch( int fd );
    int loop( int timeout=50 );

protected:
    void on_open( struct epoll_event* );
    void on_close( struct epoll_event* );
    void on_recv( struct epoll_event* );

private:
    struct epoll_event* _events;
    int _efd;
    int _max_events;
};

}
}
}

#endif
