#ifndef MEMSESS_SERVER_CONTROLLER
#define MEMSESS_SERVER_CONTROLLER

#include <arpa/inet.h>
#include <string.h>
#include <string>
#include "../interfaces/server_controller_interface.h"
#include "../interfaces/store_interface.h"
#include "../util/uuid.hpp"
#include "../util/serialization.hpp"


namespace memsess::core {
    using namespace util;

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
                const char *uuidRaw;
                const char *key;
                const char *data;
                unsigned int dataLength;
                unsigned int prolong;
                unsigned int lifetime;
                unsigned int counterKeys;
                unsigned int counterRecord;
                unsigned short int limitWrite;
                unsigned short int limitRead;
            };

            void setError( ResultCode code, Result &result );
            bool initParams( const char *data, unsigned int length, Params &params );
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

    bool ServerController::initParams( const char *data, unsigned int length, Params &params ) {

        Serialization::Item uuid;
        uuid.type = Serialization::FIXED_STRING;
        uuid.length = UUID::LENGTH_RAW;

        Serialization::Item key;
        uuid.type = Serialization::STRING_WITH_NULL;

        Serialization::Item value;
        uuid.type = Serialization::STRING;

        Serialization::Item prolong;
        prolong.type = Serialization::INT;

        Serialization::Item lifetime;
        lifetime.type = Serialization::INT;

        Serialization::Item counterKeys;
        counterKeys.type = Serialization::INT;

        Serialization::Item counterRecord;
        counterRecord.type = Serialization::INT;

        Serialization::Item limitWrite;
        limitWrite.type = Serialization::SHORT_INT;

        Serialization::Item limitRead;
        limitRead.type = Serialization::SHORT_INT;

        Serialization::Item end;
        end.type = Serialization::END;

        Serialization::Item *listGenerate[] = { &prolong, &end };
        Serialization::Item *listInit[] = { &uuid, &end };
        Serialization::Item *listProlong[] = { &uuid, &prolong, &end };
        Serialization::Item *listAddKey[] = { &uuid, &key, &prolong, &limitWrite, &limitRead, &end };
        Serialization::Item *listKey[] = { &uuid, &key, &end };
        Serialization::Item *listSetKey[] = { &uuid, &key, &value, &end };
        Serialization::Item *listProlongKey[] = { &uuid, &key, &prolong, &end };
        Serialization::Item *listSetWriteLimit[] = { &uuid, &key, &limitWrite, &end };
        Serialization::Item *listSetReadLimit[] = { &uuid, &key, &limitRead, &end };

        switch( data[0] ) {
            case Commands::GENERATE:
                if( Serialization::unpack( listGenerate, data, length - 1 ) ) {
                    return false;
                }

                params.prolong = ( unsigned int )prolong.value_int;
                break;
            case Commands::INIT:
            case Commands::REMOVE:
                if( !Serialization::unpack( listInit, data, length - 1 ) ) {
                    return false;
                }
                params.uuidRaw = uuid.value_string;
                break;
            case Commands::PROLONG:
                if( !Serialization::unpack( listProlong, data, length - 1 ) ) {
                    return false;
                }

                params.uuidRaw = uuid.value_string;
                params.prolong = ( unsigned int )prolong.value_int;
                break;
            case Commands::ADD_KEY:
                if( !Serialization::unpack( listAddKey, data, length - 1 ) ) {
                    return false;
                }

                params.uuidRaw = uuid.value_string;
                params.prolong = ( unsigned int )prolong.value_int;
                params.limitWrite = ( unsigned short int )limitWrite.value_short_int;
                params.limitRead = ( unsigned short int )limitRead.value_short_int;
                break;
            case Commands::GET_KEY:
            case Commands::REMOVE_KEY:
            case Commands::EXIST_KEY:
                if( !Serialization::unpack( listKey, data, length - 1 ) ) {
                    return false;
                }

                params.uuidRaw = uuid.value_string;
                params.key = key.value_string;
                break;
            case Commands::SET_KEY:
            case Commands::SET_FORCE_KEY:
                if( !Serialization::unpack( listSetKey, data, length - 1 ) ) {
                    return false;
                }

                params.uuidRaw = uuid.value_string;
                params.key = key.value_string;
                params.data = value.value_string;
                break;
            case Commands::PROLONG_KEY:
                if( !Serialization::unpack( listProlongKey, data, length - 1 ) ) {
                    return false;
                }

                if( ( unsigned int )prolong.value_int == 0 ) {
                    return false;
                }

                params.uuidRaw = uuid.value_string;
                params.key = key.value_string;
                params.prolong = prolong.value_int;
                break;
            case Commands::SET_LIMIT_PER_SEC_TO_READ:
                if( !Serialization::unpack( listSetReadLimit, data, length - 1 ) ) {
                    return false;
                }

                params.uuidRaw = uuid.value_string;
                params.key = key.value_string;
                params.limitRead = ( unsigned short int )limitRead.value_short_int;
                break;
            case Commands::SET_LIMIT_PER_SEC_TO_WRITE:
                if( !Serialization::unpack( listSetWriteLimit, data, length - 1 ) ) {
                    return false;
                }

                params.uuidRaw = uuid.value_string;
                params.key = key.value_string;
                params.limitWrite = ( unsigned short int )limitWrite.value_short_int;
                break;
            default:
                return false;
        }

        return true;
    }

    void ServerController::parse( const char *data, unsigned int length, Result &result ) {
        result.data = nullptr;
        result.length = 0;
        Params params;

        if( !initParams( data, length, params ) ) {
            return;
        }

        char uuid[UUID::LENGTH+1] = {};
        std::string value;

        switch( data[0] ) {
            case Commands::GENERATE:
                _store->generate( params.lifetime, uuid );
                break;
            case Commands::INIT:
                break;
            case Commands::REMOVE:
                _store->remove( params.uuidRaw );
                break;
            case Commands::PROLONG:
                _store->prolong( params.uuidRaw, params.lifetime );
                break;
            case Commands::ADD_KEY:
                _store->addKey( params.uuidRaw, params.key, params.data, params.lifetime );
                break;
            case Commands::GET_KEY:
                _store->getKey( params.uuidRaw, params.key, value, params.counterKeys, params.counterRecord );
                break;
            case Commands::REMOVE_KEY:
                _store->removeKey( params.uuidRaw, params.key );
                break;
            case Commands::EXIST_KEY:
                _store->existKey( params.uuidRaw, params.key );
                break;
            case Commands::SET_KEY:
                _store->setKey( params.uuidRaw, params.key, params.data, params.counterKeys, params.counterRecord );
                break;
            case Commands::SET_FORCE_KEY:
                break;
            case Commands::PROLONG_KEY:
                _store->prolongKey( params.uuidRaw, params.key, params.lifetime );
                break;
            case Commands::SET_LIMIT_PER_SEC_TO_READ:
                _store->setLimitToReadPerSec( params.uuidRaw, params.key, params.limitWrite );
                break;
            case Commands::SET_LIMIT_PER_SEC_TO_WRITE:
                _store->setLimitToWritePerSec( params.uuidRaw, params.key, params.limitRead );
                break;
        }
    }

    void ServerController::interval() {
        _store->clearInactive();
    }
}

#endif
