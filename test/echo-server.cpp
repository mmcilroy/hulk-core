
#include "hulk/core/tcp.h"

#include <iostream>

using namespace hulk::core;

struct echo_callback : public tcp::callback
{
    void on_open( int fd ) {
        std::cout << "on_open: " << fd << std::endl;
    }

    void on_close( int fd ) {
        std::cout << "on_close: " << fd << std::endl;
    }

    void on_recv( int fd, const char* data, size_t len ) {
        std::cout << "on_recv: " << fd << " " << len << std::endl;
    }
};

int main( int argc, char** argv )
{
    echo_callback cb;
    tcp::event_loop eloop( 1024, 5000, cb );
    eloop.watch( tcp::bind( 5555, 1024 ), true );

    while( 1 ) {
        eloop.loop();
    }
}
