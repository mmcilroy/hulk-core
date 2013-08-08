
#include "hulk/core/tcp.h"
#include "hulk/core/logger.h"

using namespace hulk::core;

log& l = logger::instance().get( "hulk.core.test" );

struct echo_callback : public tcp::callback
{
    void on_open( int fd ) {
        LOG_INFO( l, "on_open: " << fd );
    }

    void on_close( int fd ) {
        LOG_INFO( l, "on_close: " << fd );
    }

    void on_recv( int fd, const char* data, size_t len ) {
        LOG_INFO( l, "on_recv: " << fd << " " << len );
    }
};

int main( int argc, char** argv )
{
    echo_callback cb;
    tcp::event_loop eloop( 1024, 5000, cb );
    eloop.watch( tcp::bind( 5557, 1024 ), true );

    while( 1 ) {
        eloop.loop();
    }
}
