
#include "hulk/core/tcp.h"
#include "hulk/core/logger.h"

using namespace hulk;

log& l = logger::instance().get( "hulk.core.test" );

struct echo_callback : public tcp_callback
{
    void on_open( tcp_context& c ) {
        LOG_INFO( l, "on_open: " << c._fd );
    }

    void on_close( tcp_context& c) {
        LOG_INFO( l, "on_close: " << c._fd );
    }

    void on_recv( tcp_context& c, const char* data, size_t len ) {
        LOG_INFO( l, "on_recv: " << c._fd << " " << len );
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
