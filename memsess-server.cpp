#include "src/core/server_controller.hpp"
#include "src/core/store.hpp"
#include "src/core/server.hpp"
#include "src/core/cmd.hpp"
#include "src/core/monitoring.hpp"
#include "src/util/console.hpp"
#include <string>
#include <iostream>
#include <memory>
#include <thread>

using namespace memsess::util;

void startServer( memsess::core::ServerController *controller, memsess::core::Monitoring *monitoring, unsigned short int port ) {
    memsess::core::Server server( port, controller, monitoring );
    server.run();
}

void start( unsigned int limit, unsigned short int port, unsigned short int threads ) {
    std::cout << "limit " << limit << std::endl;
    std::cout << "threads " << threads << std::endl;
    std::cout << "port " << port << std::endl;


    memsess::core::Monitoring monitoring;
    memsess::core::Store store( &monitoring );
    store.setLimit( limit );

    memsess::core::ServerController controller( &store, &monitoring );

    memsess::core::Server server( port, &controller, &monitoring, true );
    memsess::util::Console::printSuccess( "Start server" );

    if( threads > 1 ) {
        for( unsigned int i = 1; i < threads; i++ ) {
            std::thread t( startServer, &controller, &monitoring, port );
            t.detach();
        }
    }

    server.run();
}

int main( int argc, char* argv[] ) {

    try {
        memsess::core::Cmd cmd( argc, argv );
        start( cmd.getLimit(), cmd.getPort(), cmd.getThreads() );
    } catch( memsess::core::Cmd::Err err ) {
        switch( err ) {
            case memsess::core::Cmd::E_WRONG_PORT:
                memsess::util::Console::printDanger( "Wrong port" );
                break;
            case memsess::core::Cmd::E_WRONG_LIMIT:
                memsess::util::Console::printDanger( "Wrong limit" );
                break;
            case memsess::core::Cmd::E_WRONG_THREADS:
                memsess::util::Console::printDanger( "Wrong threads" );
                break;
        }
    } catch( memsess::core::Server::Err err ) {
        switch( err ) {
            case memsess::core::Server::E_SERVER_ERROR:
                memsess::util::Console::printDanger( "The server could not be started" );
                break;
        }
    }


}
