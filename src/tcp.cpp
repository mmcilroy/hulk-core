
#include "hulk/core/tcp.h"

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

using namespace hulk::core::tcp;

namespace
{

int create_socket( const char* host, int port )
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

        if( host ) {
            s = ::connect( sfd, rp->ai_addr, rp->ai_addrlen );
        } else {
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

int hulk::core::tcp::bind( int port, int backlog )
{
    int fd = create_socket( 0, port );
    non_blocking( fd );

    if( ::listen( fd, backlog ) == -1 ) {
        throw std::runtime_error( "could not bind socket" );
    }

    return fd;
}

int hulk::core::tcp::connect( const char* host, int port )
{
    return create_socket( host, port );
}

int hulk::core::tcp::non_blocking( int fd )
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
    event_data( int fd, bool listening )
    : _fd( fd ), _listening( listening ) {}

    bool _listening;
    int _fd;
};

event_loop::event_loop( int max_events, callback& cb )
: _max_events( max_events ),
  _cb( cb )
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

void event_loop::watch( int fd, bool listening )
{
    struct epoll_event event;
    event.data.ptr = new event_data( fd, listening );
    event.events = EPOLLIN | EPOLLET;

    if( epoll_ctl( _efd, EPOLL_CTL_ADD, fd, &event ) == -1 ) {
        throw std::runtime_error( "could not watch fd" );
    }
}

void event_loop::dont_watch( int fd )
{
    if( epoll_ctl( _efd, EPOLL_CTL_DEL, fd, 0 ) == -1 ) {
        throw std::runtime_error( "could not stop watching fd" );
    }
}

void event_loop::on_open( struct epoll_event* e )
{
    event_data* edata = (event_data*)e->data.ptr;

    struct sockaddr in_addr;
    socklen_t len = sizeof( in_addr );
    int afd = ::accept( edata->_fd, &in_addr, &len );

    if( afd != -1 )
    {
        non_blocking( afd );
        _cb.on_open( afd );
        watch( afd, false );
    }
}

void event_loop::on_close( struct epoll_event* e )
{
    event_data* edata = (event_data*)e->data.ptr;
    dont_watch( edata->_fd );
    _cb.on_close( edata->_fd );
    ::close( edata->_fd );
    delete edata;
}

void event_loop::on_recv( struct epoll_event* e )
{
    event_data* edata = (event_data*)e->data.ptr;
    int done = 0;

    while( 1 )
    {
        char buf[512];
        ssize_t count = ::read( edata->_fd, buf, sizeof buf );

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

        _cb.on_recv( edata->_fd, buf, count );
    }

    if( done ) {
        on_close( e );
    }
}

int event_loop::loop( int timeout )
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
