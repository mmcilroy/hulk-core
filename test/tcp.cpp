
#include "hulk/core/tcp.h"
#include "hulk/core/thread.h"
#include "hulk/core/stopwatch.h"
#include "hulk/core/logger.h"

#include <sys/socket.h>

using namespace hulk;

log& l = logger::instance().get( "hulk.core.test" );

class tcp_thread : public thread
{
public:
    tcp_thread()
    : _stop( false )
    {
    }

    void stop()
    {
        _stop = true;
    }

protected:
    tcp_event_loop _eloop;

private:
    virtual void run()
    {
        while( !_stop ) {
            _eloop.loop();
        }
    }

    bool _stop;
};

struct server_callback : public tcp_callback
{
    size_t _num_bytes_recvd;

    server_callback()
    : _num_bytes_recvd( 0 )
    {
    }

    virtual void on_recv( tcp_context& c, const char* data, size_t len )
    {
        _num_bytes_recvd += len;
        ::send( c._fd, data, len, 0 );
    }
};

shared_ptr< tcp_callback > server_cb( new server_callback );

class server_thread : public tcp_thread
{
public:
    server_thread( int port )
    : tcp_thread()
    {
        _eloop.watch( tcp_bind( 5557 ), true, server_cb );
    }

    size_t num_bytes_recvd()
    {
        return ((server_callback*)server_cb.get())->_num_bytes_recvd;
    }
};

char* prepare_msg( size_t size )
{
    char* buf = new char[size];

    if( !buf ) {
        throw std::runtime_error( "prepare_buf failed" );
    }

    for( int i=0; i<size; i++ ) {
        buf[i] = i%10;
    }

    return buf;
}

int main( int argc, char** argv )
{
    size_t num_msgs = 10000;
    size_t msg_size = 512;
    char* msg_buf = prepare_msg( msg_size );

    l.set_level( log::INFO );

    server_thread server( 5557 );
    server.start();

    int fd = tcp_connect( "localhost", 5557 );
    int bs = 0;

    stopwatch watch;
    watch.start();

    for( int i=0; i<num_msgs; i++ )
    {
        ::send( fd, msg_buf, msg_size, 0 );
        bs += msg_size;
    }

    float kb = bs / 1024.0f;
    LOG_INFO( l, "client recvd " << kb << " kb. rate " << kb / watch.elapsed_s() );

    int tries = 3;
    while( bs != server.num_bytes_recvd() && tries > 0 ) {
        sleep( 1 ); --tries;
    }

    if( tries == 0 ) {
        throw std::runtime_error( "server did not receive all data" );
    }

    LOG_INFO( l, "server recvd " << server.num_bytes_recvd() / 1024.0f << " kb" );

    server.stop();
    server.join();
}
