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
#include <memory>
#include <unistd.h>
#include "../interfaces/server_controller_interface.h"
#include "../interfaces/monitoring_interface.h"
#include "../util/time.hpp"

namespace memsess::core {

    class Server {
        public:
            enum Err {
                E_SERVER_ERROR,
            };
        private:
            struct Buffer {
                unsigned int length;
                unsigned int wrLength;
                std::unique_ptr<char[]> data;
            };
            struct Connection {
                struct event* readEvent;
                struct event* writeEvent;
                unsigned long int tMonitoring;
                Buffer readBuf;
                Buffer writeBuf;
            };
            static inline i::ServerControllerInterface *_controller = nullptr;
            static inline i::MonitoringInterface *_monitoring = nullptr;

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
            static void clearBuffer( Buffer &buffer );

        public:
            Server( unsigned short int port, i::ServerControllerInterface *controller, i::MonitoringInterface *monitoring, bool isTimer = false );
            void run();
    };

    unsigned int Server::_createSocket() {
        auto sock = socket( AF_INET, SOCK_STREAM, 0 );

        if( sock == -1 ) {
            throw E_SERVER_ERROR;
        }

        auto optval = 1;
        setsockopt( sock, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof( optval ) );
        setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof( optval ) );

        auto res = fcntl( sock, F_SETFL, O_NONBLOCK );
        if( res == -1 ) {
            throw E_SERVER_ERROR;
        }

        return sock;
    }

    void Server::_bindSocket( unsigned int fd ) {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons( _port );
        addr.sin_addr.s_addr = htonl( INADDR_ANY );

        if( bind( fd, ( struct sockaddr * )&addr, sizeof( addr ) ) == -1 ) {
            throw E_SERVER_ERROR;
        }
    }

    Server::Server( unsigned short int port, i::ServerControllerInterface *controller, i::MonitoringInterface *monitoring, bool isTimer ) {
        _port = port;
        _isTimer = isTimer;
        _controller = controller;
        _monitoring = monitoring;
        _sfd = _createSocket();
        _bindSocket( _sfd );
    }

    void Server::close( int sock, Connection *conn ) {
        event_del( conn->readEvent );
        event_free( conn->readEvent );
        event_del( conn->writeEvent );
        event_free( conn->writeEvent );

        clearBuffer( conn->readBuf );
        clearBuffer( conn->writeBuf  );

        delete conn;

        ::close( sock );
    }

    void Server::read( int sock, short what, void *arg ) {
        Connection *conn = (Connection *)arg;

        if( conn->readBuf.length == 0 ) {
            conn->tMonitoring = util::Time::getMs();
            auto lengthData = 0;

            auto l = ::recv( sock, &lengthData, sizeof( unsigned int ), MSG_NOSIGNAL );

            if( l < sizeof( unsigned int ) ) {
                close( sock, conn );
                return;
            }
            _monitoring->incReceivedBytes( l );

            lengthData = ntohl( lengthData );

            if( lengthData == 0 || lengthData > 1'048'576 + 1024 ) {
                close( sock, conn );
                _monitoring->incErrorDisconnection();
                return;
            }

            conn->readBuf.length = lengthData;
            conn->readBuf.wrLength = 0;
            conn->readBuf.data = std::make_unique<char[]>( lengthData );
        }

        auto l = ::recv( sock, &conn->readBuf.data.get()[conn->readBuf.wrLength], conn->readBuf.length - conn->readBuf.wrLength, MSG_NOSIGNAL );

        if( l <= 0 ) {
            close( sock, conn );
            return;
        }

        _monitoring->incReceivedBytes( l );

        conn->readBuf.wrLength += l;

        if( conn->readBuf.wrLength == conn->readBuf.length ) {
            _monitoring->updateDurationReceiving( util::Time::getMs() - conn->tMonitoring );
            unsigned int resultLength = 0;
            auto tStart = util::Time::getMs();
            auto result = _controller->parse( conn->readBuf.data.get(), conn->readBuf.length, resultLength );

            _monitoring->updateDurationProcessing( util::Time::getMs() - tStart );
            clearBuffer( conn->readBuf );

            if( resultLength == 0 ) {
                close( sock, conn );
            } else {
                conn->writeBuf.data = std::move( result );
                conn->writeBuf.length = resultLength;
                conn->tMonitoring = util::Time::getMs();

                auto l = ::send( sock, &conn->writeBuf.data[conn->writeBuf.wrLength], conn->writeBuf.length - conn->writeBuf.wrLength, MSG_NOSIGNAL );

                if( l <= 0 ) {
                    close( sock, conn );
                    _monitoring->incErrorDisconnection();
                    return;
                }

                _monitoring->incSendedBytes( l );

                conn->writeBuf.wrLength += l;

                if( conn->writeBuf.wrLength == conn->writeBuf.length ) {
                    _monitoring->updateDurationSending( util::Time::getMs() - conn->tMonitoring );
                    clearBuffer( conn->writeBuf );
                }
            }
        }
    }

    void Server::write( int sock, short what, void *arg ) {
        Connection *conn = (Connection *)arg;

        if( conn->writeBuf.data.get() == nullptr ) {
            return;
        }

        auto l = ::send( sock, &conn->writeBuf.data[conn->writeBuf.wrLength], conn->writeBuf.length - conn->writeBuf.wrLength, MSG_NOSIGNAL );

        if( l <= 0 ) {
            close( sock, conn );
            _monitoring->incErrorDisconnection();
            return;
        }

        _monitoring->incSendedBytes( l );

        conn->writeBuf.wrLength += l;

        if( conn->writeBuf.wrLength == conn->writeBuf.length ) {
            _monitoring->updateDurationSending( util::Time::getMs() - conn->tMonitoring );
            clearBuffer( conn->writeBuf );
        }
    }

    void Server::clearBuffer( Buffer &buffer ) {
        buffer.wrLength = 0;
        buffer.length = 0;

        buffer.data.reset();
    }

    void Server::timer( int sock, short what, void *arg ) {
        auto tStart = util::Time::getMs();
        _controller->interval();
        auto tEnd = util::Time::getMs();

        _monitoring->updateDurationProcessing( tEnd - tStart );
    }

    void Server::accept( int sock, short what, void *arg) {
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
            throw E_SERVER_ERROR;
        }

        auto base = event_base_new();
        if( !base ) {
            throw E_SERVER_ERROR;
        }

        evutil_make_socket_nonblocking( _sfd );
        auto event = event_new( base, _sfd, EV_READ | EV_PERSIST, Server::accept, (void *)base );
        event_add( event, NULL );

        if( _isTimer ) {
            struct timeval time;
            time.tv_sec = 60;
            time.tv_usec = 0;

            auto ev = event_new( ( base ), -1, EV_PERSIST, Server::timer, NULL );
            evtimer_add( ev, &time );
        }

        event_base_dispatch( base );
    }
}

#endif
