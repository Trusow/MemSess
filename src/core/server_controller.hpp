#ifndef MEMSESS_SERVER_CONTROLLER
#define MEMSESS_SERVER_CONTROLLER

#include "../interfaces/server_controller_interface.h"
#include "../interfaces/store_interface.h"

namespace memsess::core {
    class ServerController: public i::ServerControllerInterface {
        private:
            i::StoreInterface *_store;
        public:
            ServerController( i::StoreInterface *store );
            const char *parse( const char *data, unsigned int length );
            void interval();
    };

    ServerController::ServerController( i::StoreInterface *store ) {
        _store = store;
    }

    const char *ServerController::parse( const char *data, unsigned int length ) {
        return nullptr;
    }

    void ServerController::interval() {
        _store->clearInactive();
    }
}

#endif
