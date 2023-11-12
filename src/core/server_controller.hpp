#ifndef MEMSESS_SERVER_CONTROLLER
#define MEMSESS_SERVER_CONTROLLER

#include <arpa/inet.h>
#include <string.h>
#include <string>
#include "../interfaces/server_controller_interface.h"
#include "../interfaces/store_interface.h"
#include "../util/uuid.hpp"

namespace memsess::core {
    class ServerController: public i::ServerControllerInterface {
        private:
            i::StoreInterface *_store;
            enum Commands {
                GENERATE = 1,
                INIT = 2,
                REMOVE = 3,
                PROLONG = 4,
                ADD_KEY = 5,
                GET_KEY = 6,
                SET_KEY = 7,
                SET_FORCE_KEY = 8,
                REMOVE_KEY = 9,
                EXIST_KEY = 10,
                PROLONG_KEY = 11,
                SET_LIMIT_PER_SEC_TO_READ = 12,
                SET_LIMIT_PER_SEC_TO_WRITE = 13,
            };
            enum ResultCode {
                OK,
                WRONG_COMMAND,
                WRONG_PARAMS,
            };
            struct Params {
                const char *uuid;
                const char *key;
                const char *data;
                unsigned int dataLength;
                unsigned int prolong;
                unsigned int lifetime;
                unsigned int counterKeys;
                unsigned int counterRecord;
                unsigned int limit;
            };

            unsigned int getLengthKey( const char *data );
            bool validate( const char *data, unsigned int length, Result &result );
            bool validateGenerate( unsigned int length );
            bool validateInit( const char *data, unsigned int length );
            bool validateRemove( const char *data, unsigned int length );
            bool validateProlong( const char *data, unsigned int length );
            bool validateAddKey( const char *data, unsigned int length );
            bool validateGetKey( const char *data, unsigned int length );
            bool validateSetKey( const char *data, unsigned int length );
            bool validateSetForceKey( const char *data, unsigned int length );
            bool validateRemoveKey( const char *data, unsigned int length );
            bool validateExistKey( const char *data, unsigned int length );
            bool validateProlongKey( const char *data, unsigned int length );
            bool validateSetLimitToRead( const char *data, unsigned int length );
            bool validateSetLimitToWrite( const char *data, unsigned int length );

            void setError( ResultCode code, Result &result );
            void initParams( const char *data, unsigned int length, Params &params );
        public:
            ServerController( i::StoreInterface *store );
            void parse( const char *data, unsigned int length, Result &result );
            void interval();
    };

    ServerController::ServerController( i::StoreInterface *store ) {
        _store = store;
    }

    void ServerController::setError( ResultCode code, Result &result ) {
        result.data = new char[1];
        result.data[0] = (char)code;
        result.length = 1;
    }

    bool ServerController::validate( const char *data, unsigned int length, Result &result ) {
        if( length == 0 ) {
            setError( ResultCode::WRONG_COMMAND, result );
            return false;
        }

        bool isValid = true;
        const char *validateData = &data[1];
        unsigned int validateDataLength = length - 1;

        switch( data[0] ) {
            case Commands::GENERATE:
                isValid = validateGenerate( validateDataLength );
                break;
            case Commands::INIT:
                isValid = validateInit( validateData, validateDataLength );
                break;
            case Commands::REMOVE:
                isValid = validateRemove( validateData, validateDataLength );
                break;
            case Commands::PROLONG:
                isValid = validateProlong( validateData, validateDataLength );
                break;
            case Commands::ADD_KEY:
                isValid = validateAddKey( validateData, validateDataLength );
                break;
            case Commands::GET_KEY:
                isValid = validateGetKey( validateData, validateDataLength );
                break;
            case Commands::SET_KEY:
                isValid = validateSetKey( validateData, validateDataLength );
                break;
            case Commands::SET_FORCE_KEY:
                isValid = validateSetForceKey( validateData, validateDataLength );
                break;
            case Commands::REMOVE_KEY:
                isValid = validateRemoveKey( validateData, validateDataLength );
                break;
            case Commands::EXIST_KEY:
                isValid = validateExistKey( validateData, validateDataLength );
                break;
            case Commands::PROLONG_KEY:
                isValid = validateProlongKey( validateData, validateDataLength );
                break;
            case Commands::SET_LIMIT_PER_SEC_TO_READ:
                isValid = validateSetLimitToRead( validateData, validateDataLength );
                break;
            case Commands::SET_LIMIT_PER_SEC_TO_WRITE:
                isValid = validateSetLimitToWrite( validateData, validateDataLength );
                break;
            default:
                setError( ResultCode::WRONG_COMMAND, result );
                return false;
        }

        if( !isValid ) {
            setError( ResultCode::WRONG_PARAMS, result );
        }

        return isValid;
    }

    unsigned int ServerController::getLengthKey( const char *data ) {
        return 0;
    }

    bool ServerController::validateGenerate( unsigned int length ) {
        return length == sizeof( unsigned int );
    }

    bool ServerController::validateInit( const char *data, unsigned int length ) {
        return length == util::UUID::LENGTH+1 && data[util::UUID::LENGTH] == 0;
    }

    bool ServerController::validateRemove( const char *data, unsigned int length ) {
        return validateInit( data, length );
    }

    bool ServerController::validateProlong( const char *data, unsigned int length ) {
        return length == util::UUID::LENGTH+1+sizeof( unsigned int ) && data[util::UUID::LENGTH] == 0;
    }

    bool ServerController::validateAddKey( const char *data, unsigned int length ) {
        getLengthKey( &data[util::UUID::LENGTH+1] );
        return length == util::UUID::LENGTH+1+sizeof( unsigned int ) * 3 && data[util::UUID::LENGTH] == 0;
    }

    bool ServerController::validateGetKey( const char *data, unsigned int length ) {
        return false;
    }

    bool ServerController::validateSetKey( const char *data, unsigned int length ) {
        return false;
    }

    bool ServerController::validateSetForceKey( const char *data, unsigned int length ) {
        return false;
    }

    bool ServerController::validateRemoveKey( const char *data, unsigned int length ) {
        return false;
    }

    bool ServerController::validateExistKey( const char *data, unsigned int length ) {
        return false;
    }

    bool ServerController::validateProlongKey( const char *data, unsigned int length ) {
        return false;
    }

    bool ServerController::validateSetLimitToRead( const char *data, unsigned int length ) {
        return false;
    }

    bool ServerController::validateSetLimitToWrite( const char *data, unsigned int length ) {
        return false;
    }

    void ServerController::initParams( const char *data, unsigned int length, Params &params ) {
        unsigned int prolong;
        switch( data[0] ) {
            case Commands::GENERATE:
                memcpy( &prolong, &data[1], sizeof( unsigned int ) );
                params.prolong = ntohl( prolong );
                break;
            case Commands::INIT:
            case Commands::REMOVE:
                params.uuid = &data[1];
                break;
            case Commands::PROLONG:
                params.uuid = &data[1];
                memcpy( &prolong, &data[1+util::UUID::LENGTH+1], sizeof( unsigned int ) );
                params.prolong = ntohl( prolong );
                break;
            case Commands::ADD_KEY:
                break;
            case Commands::GET_KEY:
            case Commands::REMOVE_KEY:
            case Commands::EXIST_KEY:
                break;
            case Commands::SET_KEY:
                break;
            case Commands::SET_FORCE_KEY:
                break;
            case Commands::PROLONG_KEY:
                break;
            case Commands::SET_LIMIT_PER_SEC_TO_READ:
            case Commands::SET_LIMIT_PER_SEC_TO_WRITE:
                break;
        }
    }

    void ServerController::parse( const char *data, unsigned int length, Result &result ) {
        result.data = nullptr;
        result.length = 0;
        Params params;

        if( !validate( data, length, result ) ) {
            return;
        }

        initParams( data, length, params );
        char *uuid;
        unsigned int counterKeys;
        unsigned int counterRecord;
        std::string value;

        switch( data[0] ) {
            case Commands::GENERATE:
                _store->generate( params.lifetime, uuid );
                break;
            case Commands::INIT:
                break;
            case Commands::REMOVE:
                _store->remove( params.uuid );
                break;
            case Commands::PROLONG:
                _store->prolong( params.uuid, params.lifetime );
                break;
            case Commands::ADD_KEY:
                _store->addKey( params.uuid, params.key, params.data, params.lifetime );
                break;
            case Commands::GET_KEY:
                _store->getKey( params.uuid, params.key, value, counterKeys, counterRecord );
                break;
            case Commands::REMOVE_KEY:
                _store->removeKey( params.uuid, params.key );
                break;
            case Commands::EXIST_KEY:
                _store->existKey( params.uuid, params.key );
                break;
            case Commands::SET_KEY:
                _store->setKey( params.uuid, params.key, params.data, params.counterKeys, params.counterRecord );
                break;
            case Commands::SET_FORCE_KEY:
                break;
            case Commands::PROLONG_KEY:
                _store->prolongKey( params.uuid, params.key, params.lifetime );
                break;
            case Commands::SET_LIMIT_PER_SEC_TO_READ:
                _store->setLimitToReadPerSec( params.uuid, params.key, params.limit );
                break;
            case Commands::SET_LIMIT_PER_SEC_TO_WRITE:
                _store->setLimitToWritePerSec( params.uuid, params.key, params.limit );
                break;
        }
    }

    void ServerController::interval() {
        _store->clearInactive();
    }
}

#endif
