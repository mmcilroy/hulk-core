
#include "hulk/core/tcp.h"
#include "hulk/core/thread.h"
#include "hulk/core/stopwatch.h"
#include "hulk/core/logger.h"

#include <cassert>
#include <cstdio>
#include <sys/socket.h>
#include <unistd.h>

using namespace hulk;

log& l = logger::instance().get( "hulk.core.test" );

const int PORT = 5558;
const int MSG_SIZE = 1024;
const int NUM_MSGS = 1000000;
const int NUM_SAMPLES = 5;

unsigned long long num_bytes_recvd = 0;

struct server_callback : public tcp_callback
{
    virtual void on_recv( tcp_context& c, const char* data, size_t len )
    {
        num_bytes_recvd += len;
    }
};

class server_thread : public thread
{
public:
    server_thread() 
    : _stop( false )
    {
        shared_ptr< tcp_callback > cb( new server_callback );
        _eloop.watch( tcp_bind( PORT ), true, cb );
    }

    void stop()
    {
        _stop = true;
    }

private:
    virtual void run()
    {
        while( !_stop ) {
            _eloop.loop();
        }
    }

    tcp_event_loop _eloop;
    bool _stop;
};

class client_thread : public thread
{
public:
    client_thread()
    {
        _fd = tcp_connect( "localhost", PORT );
    }

private:
    virtual void run()
    {
        char* buf = new char[MSG_SIZE];

        for( int i=0; i<NUM_MSGS; i++ )
        {
            if( ::send( _fd, buf, MSG_SIZE, 0 ) == -1 ) {
                perror( "send failed" ); break;
            }
        }

        delete buf;

        ::close( _fd );
    }

    int _fd;
};

int main( int argc, char** argv )
{
    // launch our server
    server_thread server;
    server.start();

    for( int i=0; i<NUM_SAMPLES; i++ )
    {
        num_bytes_recvd = 0;

        client_thread client;

        stopwatch watch;
        watch.start();

        client.start();
        client.join();

        while( num_bytes_recvd != NUM_MSGS * MSG_SIZE ) {
            thread::yield();
        }

        float secs = watch.elapsed_s();
        float mb = ( NUM_MSGS * MSG_SIZE ) / 1024.0f / 1024.0f;
        std::cout << mb << " MB in " << secs << " seconds. rate " << mb / secs << " MB/s\n";
    }

    server.stop();
    server.join();
}
