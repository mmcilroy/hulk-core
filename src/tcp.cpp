
#include "hulk/core/tcp.h"
#include "hulk/core/logger.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <cstring>
#include <stdexcept>
#include <sstream>

using namespace hulk;

namespace
{

log& l = logger::instance().get( "hulk.core.tcp" );

int tcp_create_socket( const char* host, int port )
{
    struct addrinfo hints;
    memset( &hints, 0, sizeof( struct addrinfo ) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    std::stringstream ss;
    ss << port;
    struct addrinfo* result;
    int s = getaddrinfo( host, ss.str().c_str(), &hints, &result );

    if( s != 0 ) {
        throw std::runtime_error( gai_strerror( s ) );
    }

    struct addrinfo* rp;
    int sfd;

    for( rp = result; rp; rp = rp->ai_next )
    {
        sfd = socket( rp->ai_family, rp->ai_socktype, rp->ai_protocol );

        if( sfd == -1 ) {
            continue;
        }

        if( host )
        {
            s = ::connect( sfd, rp->ai_addr, rp->ai_addrlen );
        }
        else
        {
            int o = 1;
            ::setsockopt( sfd, SOL_SOCKET, SO_REUSEADDR, &o, sizeof( int ) );
            s = ::bind( sfd, rp->ai_addr, rp->ai_addrlen );
        }

        if( s == 0 ) {
            break;
        } else {
            close( sfd );
        }
    }

    if( !rp ) {
        throw std::runtime_error( "could not create socket" );
    }

    freeaddrinfo( result );

    return sfd;
}

}

int hulk::tcp_bind( int port, int backlog )
{
    LOG_DEBUG( l, "tcp_bind: port=" << port << ", backlog=" << backlog );

    int fd = tcp_create_socket( 0, port );
    tcp_non_blocking( fd );

    if( ::listen( fd, backlog ) == -1 ) {
        throw std::runtime_error( "could not bind socket" );
    }

    return fd;
}

int hulk::tcp_connect( const char* host, int port )
{
    LOG_DEBUG( l, "tcp_connect: host=" << host << ", port=" << port );

    return tcp_create_socket( host, port );
}

int hulk::tcp_non_blocking( int fd )
{
    int flags = fcntl( fd, F_GETFL, 0 );

    if( flags == -1 ) {
        throw std::runtime_error( "fcntl get failed" );
    }

    if( fcntl( fd, F_SETFL, flags | O_NONBLOCK ) == -1 ) {
        throw std::runtime_error( "fcntl set failed" );
    }

    return fd;
}

struct hulk::event_data
{
    event_data( int fd, bool listening, const shared_ptr< tcp_callback >& cb )
    : _cb( cb ),
      _listening( listening )
    {
        LOG_DEBUG( l, "new event_data @ " << this << ", fd " << fd );

        _context._fd = fd;
        _context._data = 0;
    }

    ~event_data()
    {
        LOG_DEBUG( l, "del event_data @ " << this );
    }

    tcp_context _context;
    shared_ptr< tcp_callback > _cb;
    bool _listening;
};

tcp_event_loop::tcp_event_loop( int max_events, int max_recv_buf )
: _max_events( max_events ),
  _max_recv_buf( max_recv_buf )
{
    _efd = epoll_create1( 0 );
    if( _efd == -1 ) {
        throw std::runtime_error( "could not create epoll fd" );
    }

    _events = ( struct epoll_event* )calloc( max_events, sizeof( struct epoll_event ) );
    if( _events == 0 ) {
        throw std::runtime_error( "could not create epoll events" );
    }

    _recv_buf = new char[max_recv_buf];
    if( _recv_buf == 0 ) {
        throw std::runtime_error( "could not create recv buffer" );
    }
}

tcp_event_loop::~tcp_event_loop()
{
    if( _efd ) {
        ::close( _efd );
    }

    if( _events ) {
        ::free( _events );
    }

    if( _recv_buf ) {
        delete [] _recv_buf;
    }

    std::set< event_data* >::const_iterator it = _event_data.begin();
    for( ; it != _event_data.end(); ++it )
    {
        event_data* edata = (*it);
        if( edata )
        {
            ::close( edata->_context._fd );
            delete edata;
        }
    }
}

int tcp_event_loop::watch( int fd, bool listening, const shared_ptr< tcp_callback >& cb )
{
    LOG_DEBUG( l, "watch: fd=" << fd << ", listening=" << ( listening ? "y" : "n" ) );

    event_data* edata = new event_data( fd, listening, cb );
    _event_data.insert( edata );

    struct epoll_event event;
    event.data.ptr = edata;
    event.events = EPOLLIN | EPOLLET;

    if( epoll_ctl( _efd, EPOLL_CTL_ADD, fd, &event ) == -1 ) {
        throw std::runtime_error( "could not watch fd" );
    }

    if( !listening ) {
        cb->on_open( edata->_context );
    }

    return fd;
}

void tcp_event_loop::on_open( struct epoll_event* e )
{
    event_data* edata = (event_data*)e->data.ptr;

    LOG_DEBUG( l, "on_open: fd=" << edata->_context._fd );

    struct sockaddr in_addr;
    socklen_t len = sizeof( in_addr );

    int afd;
    while( ( afd = ::accept( edata->_context._fd, &in_addr, &len ) ) != -1 )
    {
        tcp_non_blocking( afd );
        watch( afd, false, edata->_cb );
    }
}

void tcp_event_loop::on_close( struct epoll_event* e )
{
    event_data* edata = (event_data*)e->data.ptr;

    if( edata )
    {
        LOG_DEBUG( l, "on_close: fd=" << edata->_context._fd );

        edata->_cb->on_close( edata->_context );

        int err = epoll_ctl( _efd, EPOLL_CTL_DEL, edata->_context._fd, 0 );
        if( err == -1 ) {
            LOG_ERROR( l, "epoll_ctl del failed: " << strerror( err ) );
        }

        _event_data.erase( edata );
        delete edata;
    }
    else
    {
        LOG_ERROR( l, "on_close: null edata" );
    }
}

void tcp_event_loop::on_recv( struct epoll_event* e )
{
    event_data* edata = (event_data*)e->data.ptr;

    LOG_DEBUG( l, "on_recv: fd=" << edata->_context._fd );

    int done = 0;

    while( 1 )
    {
        ssize_t count = ::read( edata->_context._fd, _recv_buf, _max_recv_buf );

        if( count == -1 )
        {
            if( errno != EAGAIN ) {
                done = 1;
            }
            break;
        }
        else
        if( count == 0 )
        {
            done = 1;
            break;
        }

        edata->_cb->on_recv( edata->_context, _recv_buf, count );
    }

    if( done ) {
        on_close( e );
    }
}

int tcp_event_loop::loop( int timeout )
{
    int n = epoll_wait( _efd, _events, _max_events, timeout );

    for( int i = 0; i < n; i++ )
    {
        event_data* edata = (event_data*)_events[i].data.ptr;
        if( edata )
        {
            uint32_t events = _events[i].events;
            if( events & EPOLLERR || events & EPOLLHUP || !( events & EPOLLIN ) ) {
                on_close( &_events[i] );
            } else if( edata->_listening ) {
                on_open( &_events[i] );
            } else {
                on_recv( &_events[i] );
            }
        }
    }

    return n;
}
