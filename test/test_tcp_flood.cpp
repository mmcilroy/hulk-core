
#include "hulk/core/tcp.h"
#include "hulk/core/thread.h"
#include "hulk/core/logger.h"

#include <cassert>
#include <cstdio>
#include <sys/socket.h>
#include <unistd.h>

using namespace hulk;

log& l = logger::instance().get( "hulk.core.test" );

const int PORT = 5558;
const int NUM_CLIENTS = 64;
const int NUM_MSGS = 10000;
const int MSG_SIZE = 1024;

int num_clients_connected = 0;
unsigned long long num_bytes_recvd = 0;
unsigned long long answer = 0;

struct server_callback : public tcp_callback
{
    virtual void on_open( tcp_context& )
    {
        ++num_clients_connected;
    }

    virtual void on_close( tcp_context& )
    {
        --num_clients_connected;
    }

    virtual void on_recv( tcp_context& c, const char* data, size_t len )
    {
        num_bytes_recvd += len;

        for( int i=0; i<len; i++ ) {
            answer += (unsigned long)data[i];
        }
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
    client_thread( int num_msgs, int msg_size )
    : _num_msgs( num_msgs ),
      _msg_size( msg_size )
    {
        _fd = tcp_connect( "localhost", PORT );
    }

private:
    char* prepare_buf( int size )
    {
        char* buf = new char[size];

        if( !buf ) {
            throw std::runtime_error( "prepare_buf failed" );
        }

        for( int i=0; i<size; i++ ) {
            buf[i] = i%2;
        }

        return buf;
    }

    virtual void run()
    {
        char* buf = prepare_buf( _msg_size );

        for( int i=0; i<_num_msgs; i++ )
        {
            if( ::send( _fd, buf, _msg_size, 0 ) == -1 ) {
                perror( "send failed" ); break;
            }
        }

        delete [] buf;

        ::close( _fd );
    }

    int _fd;
    int _msg_size;
    int _num_msgs;
};

int main( int argc, char** argv )
{
    // launch our server
    server_thread server;
    server.start();

    client_thread* clients[NUM_CLIENTS];

    // create clients
    for( int i=0; i<NUM_CLIENTS; i++ ) {
        clients[i] = new client_thread( NUM_MSGS, MSG_SIZE );
    }

    // at this point all clients should have established a connection to the server
    thread::sleep( 1000 );
    std::cout << num_clients_connected << " clients connected\n";
    assert( num_clients_connected == NUM_CLIENTS );

    // tell clients to start sending data
    for( int i=0; i<NUM_CLIENTS; i++ ) {
        clients[i]->start();
    }

    // wait for client threads to complete
    for( int i=0; i<NUM_CLIENTS; i++ ) {
        clients[i]->join();
    }

    // all clients should have disconnected and the server should have received all data
    thread::sleep( 1000 );
    std::cout << "server processed " << num_bytes_recvd / 1024.0f / 1024.0f << " MB, answer = " << answer << "\n";
    assert( num_clients_connected == 0 );
    assert( num_bytes_recvd == NUM_MSGS * MSG_SIZE * NUM_CLIENTS );
    assert( answer == num_bytes_recvd / 2 );

    // bring server down and cleanup
    server.stop();
    server.join();

    for( int i=0; i<NUM_CLIENTS; i++ ) {
        delete clients[i];
    }
}
