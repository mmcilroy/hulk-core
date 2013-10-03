
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
        throw std::runtime_error( "fcntl() failed F_GETFL" );
    }

    if( fcntl( fd, F_SETFL, flags | O_NONBLOCK ) == -1 ) {
        throw std::runtime_error( "fcntl() failed F_SETFL" );
    }

    return fd;
}

struct event_data
{
    event_data( int fd, bool listening, tcp_callback& cb )
    : _cb( cb ),
      _listening( listening )
    {
        _context._fd = fd;
        _context._data = 0;
    }

    tcp_context _context;
    tcp_callback& _cb;
    bool _listening;
};

tcp_event_loop::tcp_event_loop( int max_events )
: _max_events( max_events )
{
    _efd = epoll_create1( 0 );
    if( _efd == -1 ) {
        throw std::runtime_error( "could not create epoll fd" );
    }

    _events = ( struct epoll_event* )calloc( max_events, sizeof( struct epoll_event ) );
    if( _events == 0 ) {
        throw std::runtime_error( "could not create epoll events" );
    }
}

tcp_event_loop::~tcp_event_loop()
{
    ::close( _efd );
    ::free( _events );
}

void tcp_event_loop::watch( int fd, bool listening, tcp_callback& cb )
{
    LOG_DEBUG( l, "watch: fd=" << fd << ", listening=" << ( listening ? "y" : "n" ) );

    struct epoll_event event;
    event_data* edata = new event_data( fd, listening, cb );
    event.data.ptr = edata;
    event.events = EPOLLIN | EPOLLET;

    if( epoll_ctl( _efd, EPOLL_CTL_ADD, fd, &event ) == -1 ) {
        throw std::runtime_error( "could not watch fd" );
    }

    if( !listening ) {
        cb.on_open( edata->_context );
    }
}

void tcp_event_loop::dont_watch( int fd )
{
    LOG_DEBUG( l, "dont_watch: fd=" << fd );

    if( epoll_ctl( _efd, EPOLL_CTL_DEL, fd, 0 ) == -1 ) {
        throw std::runtime_error( "could not stop watching fd" );
    }
}

void tcp_event_loop::on_open( struct epoll_event* e )
{
    event_data* edata = (event_data*)e->data.ptr;

    LOG_DEBUG( l, "on_open: fd=" << edata->_context._fd );

    struct sockaddr in_addr;
    socklen_t len = sizeof( in_addr );
    int afd = ::accept( edata->_context._fd, &in_addr, &len );

    if( afd != -1 )
    {
        tcp_non_blocking( afd );
        watch( afd, false, edata->_cb );
    }
}

void tcp_event_loop::on_close( struct epoll_event* e )
{
    event_data* edata = (event_data*)e->data.ptr;

    LOG_DEBUG( l, "on_close: fd=" << edata->_context._fd );

    dont_watch( edata->_context._fd );
    edata->_cb.on_close( edata->_context );
    ::close( edata->_context._fd );
    delete edata;
}

void tcp_event_loop::on_recv( struct epoll_event* e )
{
    event_data* edata = (event_data*)e->data.ptr;

    LOG_DEBUG( l, "on_recv: fd=" << edata->_context._fd );

    int done = 0;

    while( 1 )
    {
        char buf[512];
        ssize_t count = ::read( edata->_context._fd, buf, sizeof buf );

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

        edata->_cb.on_recv( edata->_context, buf, count );
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
