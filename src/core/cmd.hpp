#ifndef MEMSESS_CORE_CMD
#define MEMSESS_CORE_CMD

#include <string>
#include <stdlib.h>
#include <thread>

namespace memsess::core {
    class Cmd {
        public:
            enum Err {
                E_WRONG_PORT,
                E_WRONG_LIMIT,
                E_WRONG_THREADS,
            };
        private:
            enum CMD {
                CMD_LIMIT,
                CMD_PORT,
                CMD_THREADS,
                CMD_UNKNOWN,
            };

            const unsigned int _defaultThreads = std::thread::hardware_concurrency();

            unsigned int _limit = 0xFFFFFFFF;
#if MEMSESS_MULTI
            unsigned short int _threads = _defaultThreads;
#else
            unsigned short int _threads = 1;
#endif
            unsigned short int _port = 2901;

            CMD _getCommand( const char *value );

            unsigned int _getLimit( const char *value );
            unsigned short int _getPort( const char *value );
            unsigned short int _getThreads( const char *value );

        public:
            Cmd( int argc, char* argv[] );
            unsigned int getLimit();
            unsigned int getThreads();
            unsigned int getPort();
    };

    Cmd::Cmd( int argc, char* argv[] ) {
        CMD cmd = CMD_UNKNOWN;

        for( int i = 1; i < argc; i++ ) {
            const char *value = argv[i];
            if( cmd == CMD_UNKNOWN ) {
                cmd = _getCommand( value );
            } else {
                switch( cmd ) {
                    case CMD_LIMIT:
                        _limit = _getLimit( value );
                        break;
                    case CMD_PORT:
                        _port = _getPort( value );
                        break;
#if MEMSESS_MULTI
                    case CMD_THREADS:
                        _threads = _getThreads( value );
                        break;
#endif
                }

                cmd = CMD_UNKNOWN;
            }
        }
    }

    Cmd::CMD Cmd::_getCommand( const char *value ) {
        auto str = std::string( value );

        if( str == "-l" ) {
            return CMD_LIMIT;
        } else if( str == "-p" ) {
            return CMD_PORT;
        } else if( str == "-t" ) {
            return CMD_THREADS;
        }

        return CMD_UNKNOWN;
    }

    unsigned int Cmd::_getLimit( const char *value ) {
        auto v = atoi( value );

        if( v == 0 ) {
            throw E_WRONG_LIMIT;
        }

        return v;
    }

    unsigned short int Cmd::_getPort( const char *value ) {
        auto v = atoi( value );

        if( v == 0 || v > 0xFFFF ) {
            throw E_WRONG_PORT;
        }

        return v;
    }

    unsigned short int Cmd::_getThreads( const char *value ) {
        auto v = atoi( value );

        if( v == 0 || v > _defaultThreads ) {
            throw E_WRONG_THREADS;
        }

        return v;
    }

    unsigned int Cmd::getLimit() {
        return _limit;
    }

    unsigned int Cmd::getPort() {
        return _port;
    }

    unsigned int Cmd::getThreads() {
        return _threads;
    }
}

#endif
