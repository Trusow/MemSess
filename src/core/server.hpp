#ifndef MEMSESS_CORE_SERVER
#define MEMSESS_CORE_SERVER

#include <event2/event.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <random>
#include <iostream>
#include <unistd.h>
#include "../interfaces/server_controller_interface.h"

namespace memsess::core {

    class Server {
        private:
            struct Buffer {
                unsigned int length;
                unsigned int wrLength;
                char *data;
            };
            struct Connection {
                struct event* readEvent;
                struct event* writeEvent;
                Buffer readBuf;
                Buffer writeBuf;
            };
            static inline i::ServerControllerInterface *_controller = nullptr;

            unsigned int _createSocket();
            void _bindSocket( unsigned int fd );

            bool _isTimer = false;

            unsigned int _sfd;
            unsigned short int _port;
            const unsigned int COUNT_LISTEN = 512;
            static void accept( int sock, short what, void *base );
            static void read( int sock, short what, void *arg );
            static void write( int sock, short what, void *arg );
            static void close( int sock, Connection *conn );
            static void timer( int sock, short what, void *arg );

        public:
            Server( unsigned short int port, i::ServerControllerInterface *controller, bool isTimer = false );
            void run();
    };

    unsigned int Server::_createSocket() {
        auto sock = socket( AF_INET, SOCK_STREAM, 0 );

        if( sock == -1 ) {
            throw -1;
        }

        auto optval = 1;
        setsockopt( sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof( optval ) );
        setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof( optval ) );

        auto res = fcntl( sock, F_SETFL, O_NONBLOCK );
        if( res == -1 ) {
            throw -1;
        }

        return sock;
    }

    void Server::_bindSocket( unsigned int fd ) {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons( _port );
        addr.sin_addr.s_addr = htonl( INADDR_ANY );

        if( bind( fd, ( struct sockaddr * )&addr, sizeof( addr ) ) == -1 ) {
            throw -1;
        }
    }

    Server::Server( unsigned short int port, i::ServerControllerInterface *controller, bool isTimer ) {
        _port = port;
        _isTimer = isTimer;
        _controller = controller;
        _sfd = _createSocket();
        _bindSocket( _sfd );
    }

    void Server::close( int sock, Connection *conn ) {
        event_del( conn->readEvent );
        event_free( conn->readEvent );
        event_del( conn->writeEvent );
        event_free( conn->writeEvent );

        if( conn->readBuf.data != nullptr ) {
            delete[] conn->readBuf.data;
        }

        if( conn->writeBuf.data != nullptr ) {
            delete[] conn->writeBuf.data;
        }

        delete conn;

        ::close( sock );
    }

    void Server::read( int sock, short what, void *arg ) {
        Connection *conn = (Connection *)arg;
        auto buf = conn->readBuf;

        if( buf.length == 0 ) {
            auto lengthData = 0;

            auto l = ::read( sock, &lengthData, sizeof( unsigned int ) );
            if( l < sizeof( unsigned int ) ) {
                close( sock, conn );
                return;
            }

            lengthData = ntohl( lengthData );

            if( lengthData == 0 || lengthData > 1'048'576 + 1024 ) {
                close( sock, conn );
                return;
            }

            buf.length = lengthData;
            buf.data = new char[lengthData];
        }

        auto l = ::read( sock, &buf.data[buf.wrLength], buf.length - buf.wrLength );

        if( l <= 0 ) {
            close( sock, conn );
            return;
        }

        buf.wrLength += l;

        if( buf.wrLength == buf.length ) {
            _controller->parse( buf.data, buf.length );
            buf.wrLength = 0;
            buf.length = 0;
            // callback
            delete[] buf.data;
            buf.data = nullptr;
        }
    }

    void Server::write( int sock, short what, void *arg ) {
        std::cout << "write" << std::endl;
    }

    void Server::timer( int sock, short what, void *arg ) {
        _controller->interval();

        std::cout << "timer" << std::endl;
    }

    void Server::accept( int sock, short what, void *arg) {
        //std::cout << "new connect " << sock << std::endl;
        auto fd = ::accept( sock, 0, 0 );

        if( fd < 0 ) {
            return;
        }

        fcntl( fd, F_SETFL, O_NONBLOCK );
        evutil_make_socket_nonblocking( fd );

        auto optval = 1;
        setsockopt( fd, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof( optval ) );
        optval = 0;
        setsockopt( fd, IPPROTO_TCP, TCP_CORK, &optval, sizeof( optval ) );

        Connection *conn = new Connection{};

        conn->readEvent = event_new( (event_base *)arg, fd, EV_READ | EV_PERSIST, Server::read, conn );
        conn->writeEvent = event_new( (event_base *)arg, fd, EV_WRITE, Server::write, conn );
        event_add( conn->readEvent, NULL );
        event_add( conn->writeEvent, NULL );
    }

    void Server::run() {
        if( listen( _sfd, COUNT_LISTEN ) == -1 ) {
            throw -1;
        }

        auto base = event_base_new();
        if( !base ) {
            throw -1;
        }

        evutil_make_socket_nonblocking( _sfd );
        auto event = event_new( base, _sfd, EV_READ | EV_PERSIST, Server::accept, (void *)base );
        event_add( event, NULL );

        if( _isTimer ) {
            struct timeval time;
            time.tv_sec = 1;
            time.tv_usec = 0;

            auto ev = event_new( ( base ), -1, EV_PERSIST, Server::timer, NULL );
            evtimer_add( ev, &time );
        }

        event_base_dispatch( base );
    }
}

#endif
