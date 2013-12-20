
#include "hulk/core/tcp.h"
#include "hulk/core/logger.h"
#include <unistd.h>
#include <iostream>

using namespace hulk;

log& l = logger::instance().get( "hulk.core.test" );

const int PORT = 5558;

int main( int argc, char** argv )
{
    shared_ptr< tcp_callback > cb( new tcp_callback );

    tcp_event_loop eloop;

    int sfd = tcp_bind( PORT );
    int cfd = tcp_connect( "localhost", PORT );

    eloop.watch( sfd, true, cb );
    eloop.watch( cfd, false, cb );
    eloop.loop();

    ::close( cfd );
    ::close( sfd );

    eloop.loop();
}
