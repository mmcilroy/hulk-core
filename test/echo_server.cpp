
#include "hulk/core/tcp.h"
#include <iostream>
#include <cstring>

#include <sys/socket.h>

using namespace hulk;

struct echo_callback : public tcp_callback
{
    void on_open( tcp_context& c )
    {
        std::cout << "on_open: " << c._fd << std::endl;
    }

    void on_close( tcp_context& c )
    {
        std::cout << "on_close: " << c._fd << std::endl;
    }

    void on_recv( tcp_context& c, const char* data, size_t len )
    {
        char* s = new char[len+1];
        strncpy( s, data, len );
        s[len] = 0;

        std::cout << "on_recv: " << s << std::endl;

        const char* m = "<html><body><h1>hi</h1></body></html>";
        ::send( c._fd, m, strlen( m ), 0 );

        delete s;
    }
};

int main( int argc, char** argv )
{
    shared_ptr< tcp_callback > cb( new echo_callback );

    tcp_event_loop eloop;
    eloop.watch( tcp_bind( atoi( argv[1] ) ), true, cb );

    while( 1 ) {
        eloop.loop( 5000 );
    }
}
