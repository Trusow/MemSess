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
    using namespace i;

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
                ALL_ADD_KEY = 14,
                ALL_REMOVE_KEY = 15,
                ALL_SET_LIMIT_PER_SEC_TO_READ = 16,
                ALL_SET_LIMIT_PER_SEC_TO_WRITE = 17,
            };
            enum ResultCode {
                OK = 1,
                WRONG_COMMAND = 2,
                WRONG_PARAMS = 3,
                SESSION_NONE = 4,
                KEY_NONE = 5,
                LIMIT = 6,
                LIFETIME = 7,
                DUPLICATE_KEY = 8,
                RECORD_BEEN_CHANGED = 9,
                LIMIT_PER_SEC = 10,
            };
            struct Params {
                const char *uuidRaw;
                const char *key;
                const char *data;
                unsigned int dataLength;
                unsigned int lifetime;
                unsigned int counterKeys;
                unsigned int counterRecord;
                unsigned short int limitWrite;
                unsigned short int limitRead;
            };

            bool initParams( const char *data, unsigned int length, Params &params );
            bool initCmd( char cmd );
            ResultCode convertStoreError( StoreInterface::Result error );
        public:
            ServerController( i::StoreInterface *store );
            std::unique_ptr<char[]> parse(
                const char *data,
                unsigned int length,
                unsigned int &resultLength
            );
            void interval();
    };

    ServerController::ServerController( i::StoreInterface *store ) {
        _store = store;
    }

    bool ServerController::initCmd( char cmd ) {
        switch( cmd ) {
            case GENERATE:
            case INIT:
            case REMOVE:
            case PROLONG:
            case ADD_KEY:
            case GET_KEY:
            case SET_KEY:
            case SET_FORCE_KEY:
            case REMOVE_KEY:
            case EXIST_KEY:
            case PROLONG_KEY:
            case SET_LIMIT_PER_SEC_TO_READ:
            case SET_LIMIT_PER_SEC_TO_WRITE:
            case ALL_ADD_KEY:
            case ALL_REMOVE_KEY:
            case ALL_SET_LIMIT_PER_SEC_TO_READ:
            case ALL_SET_LIMIT_PER_SEC_TO_WRITE:
                return true;
            default:
                return false;
        }
    }

    ServerController::ResultCode ServerController::convertStoreError( StoreInterface::Result error ) {
        switch( error ) {
            case StoreInterface::E_SESSION_NONE:
                return SESSION_NONE;
            case StoreInterface::E_KEY_NONE:
                return KEY_NONE;
            case StoreInterface::E_LIMIT:
                return LIMIT;
            case StoreInterface::E_LIFETIME:
                return LIFETIME;
            case StoreInterface::E_DUPLICATE_KEY:
                return DUPLICATE_KEY;
            case StoreInterface::E_RECORD_BEEN_CHANGED:
                return RECORD_BEEN_CHANGED;
            case StoreInterface::E_LIMIT_PER_SEC:
                return LIMIT_PER_SEC;
            default:
                return OK;
        }
    }

    bool ServerController::initParams( const char *data, unsigned int length, Params &params ) {

        Serialization::Item uuid;
        uuid.type = Serialization::FIXED_STRING;
        uuid.length = UUID::LENGTH_RAW;

        Serialization::Item key;
        key.type = Serialization::STRING_WITH_NULL;

        Serialization::Item value;
        value.type = Serialization::STRING;

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
        Serialization::Item *listAddKey[] = { &uuid, &key, &value, &prolong, &limitRead, &limitWrite, &end };
        Serialization::Item *listKey[] = { &uuid, &key, &end };
        Serialization::Item *listSetKey[] = { &uuid, &key, &value, &counterKeys, &counterRecord, &end };
        Serialization::Item *listSetForceKey[] = { &uuid, &key, &value, &end };
        Serialization::Item *listProlongKey[] = { &uuid, &key, &prolong, &end };
        Serialization::Item *listSetWriteLimit[] = { &uuid, &key, &limitWrite, &end };
        Serialization::Item *listSetReadLimit[] = { &uuid, &key, &limitRead, &end };
        Serialization::Item *listAllAddKey[] = { &key, &value, &limitRead, &limitWrite, &end };
        Serialization::Item *listAllRemoveKey[] = { &key, &end };
        Serialization::Item *listAllSetWriteLimit[] = { &key, &limitWrite, &end };
        Serialization::Item *listAllSetReadLimit[] = { &key, &limitRead, &end };

        switch( data[0] ) {
            case Commands::GENERATE:
                if( !Serialization::unpack( listGenerate, &data[1], length - 1 ) ) {
                    return false;
                }

                params.lifetime = ( unsigned int )prolong.value_int;
                break;
            case Commands::INIT:
            case Commands::REMOVE:
                if( !Serialization::unpack( listInit, &data[1], length - 1 ) ) {
                    return false;
                }
                params.uuidRaw = uuid.value_string;
                break;
            case Commands::PROLONG:
                if( !Serialization::unpack( listProlong, &data[1], length - 1 ) ) {
                    return false;
                }

                params.uuidRaw = uuid.value_string;
                params.lifetime = ( unsigned int )prolong.value_int;
                break;
            case Commands::ADD_KEY:
                if( !Serialization::unpack( listAddKey, &data[1], length - 1 ) ) {
                    return false;
                }


                params.key = key.value_string;
                params.data = value.value_string;
                params.uuidRaw = uuid.value_string;
                params.lifetime = ( unsigned int )prolong.value_int;
                params.limitWrite = ( unsigned short int )limitWrite.value_short_int;
                params.limitRead = ( unsigned short int )limitRead.value_short_int;
                break;
            case Commands::ALL_ADD_KEY:
                if( !Serialization::unpack( listAllAddKey, &data[1], length - 1 ) ) {
                    return false;
                }


                params.key = key.value_string;
                params.data = value.value_string;
                params.limitWrite = ( unsigned short int )limitWrite.value_short_int;
                params.limitRead = ( unsigned short int )limitRead.value_short_int;
                break;
            case Commands::ALL_REMOVE_KEY:
                if( !Serialization::unpack( listAllRemoveKey, &data[1], length - 1 ) ) {
                    return false;
                }


                params.key = key.value_string;
                break;
            case Commands::GET_KEY:
            case Commands::REMOVE_KEY:
            case Commands::EXIST_KEY:
                if( !Serialization::unpack( listKey, &data[1], length - 1 ) ) {
                    return false;
                }


                params.uuidRaw = uuid.value_string;
                params.key = key.value_string;
                break;
            case Commands::SET_KEY:
                if( !Serialization::unpack( listSetKey, &data[1], length - 1 ) ) {
                    return false;
                }

                params.uuidRaw = uuid.value_string;
                params.key = key.value_string;
                params.data = value.value_string;
                params.dataLength = value.length;
                params.counterKeys = counterKeys.value_int;
                params.counterRecord = counterRecord.value_int;
                break;
            case Commands::SET_FORCE_KEY:
                if( !Serialization::unpack( listSetForceKey, &data[1], length - 1 ) ) {
                    return false;
                }

                params.uuidRaw = uuid.value_string;
                params.key = key.value_string;
                params.data = value.value_string;
                params.dataLength = value.length;
                break;
            case Commands::PROLONG_KEY:
                if( !Serialization::unpack( listProlongKey, &data[1], length - 1 ) ) {
                    return false;
                }

                if( ( unsigned int )prolong.value_int == 0 ) {
                    return false;
                }

                params.uuidRaw = uuid.value_string;
                params.key = key.value_string;
                params.lifetime = prolong.value_int;
                break;
            case Commands::SET_LIMIT_PER_SEC_TO_READ:
                if( !Serialization::unpack( listSetReadLimit, &data[1], length - 1 ) ) {
                    return false;
                }

                params.uuidRaw = uuid.value_string;
                params.key = key.value_string;
                params.limitRead = ( unsigned short int )limitRead.value_short_int;
                break;
            case Commands::ALL_SET_LIMIT_PER_SEC_TO_READ:
                if( !Serialization::unpack( listAllSetReadLimit, &data[1], length - 1 ) ) {
                    return false;
                }

                params.key = key.value_string;
                params.limitRead = ( unsigned short int )limitRead.value_short_int;
                break;
            case Commands::SET_LIMIT_PER_SEC_TO_WRITE:
                if( !Serialization::unpack( listSetWriteLimit, &data[1], length - 1 ) ) {
                    return false;
                }

                params.uuidRaw = uuid.value_string;
                params.key = key.value_string;
                params.limitWrite = ( unsigned short int )limitWrite.value_short_int;
                break;
            case Commands::ALL_SET_LIMIT_PER_SEC_TO_WRITE:
                if( !Serialization::unpack( listAllSetWriteLimit, &data[1], length - 1 ) ) {
                    return false;
                }

                params.key = key.value_string;
                params.limitWrite = ( unsigned short int )limitWrite.value_short_int;
                break;
            default:
                return false;
        }

        return true;
    }

    std::unique_ptr<char[]> ServerController::parse(
        const char *data,
        unsigned int length,
        unsigned int &resultLength
    ) {
        Params params;
        resultLength = 0;

        unsigned char cmd = data[0];

        char uuid[UUID::LENGTH+1] = {};
        char uuidRaw[UUID::LENGTH_RAW] = {};
        std::string value;
        unsigned int counterKeys;
        unsigned int counterRecord;

        StoreInterface::Result res = StoreInterface::OK;

        Serialization::Item itemResult;
        itemResult.type = Serialization::CHAR;

        Serialization::Item itemUUID;
        itemUUID.type = Serialization::FIXED_STRING;
        itemUUID.length = UUID::LENGTH_RAW;

        Serialization::Item itemValue;
        itemValue.type = Serialization::STRING;

        Serialization::Item itemValueFinal;
        itemValueFinal.type = Serialization::STRING;

        Serialization::Item itemCounterKeys;
        itemCounterKeys.type = Serialization::INT;

        Serialization::Item itemCounterRecord;
        itemCounterRecord.type = Serialization::INT;

        Serialization::Item itemEnd;
        itemEnd.type = Serialization::END;

        Serialization::Item *listNone[] = { &itemResult, &itemEnd };
        Serialization::Item *listGenerate[] = { &itemResult, &itemUUID, &itemEnd };
        Serialization::Item *listAddKey[] = { &itemResult, &itemCounterKeys, &itemCounterRecord, &itemEnd };
        Serialization::Item *listGetKey[] = { &itemResult, &itemValue, &itemCounterKeys, &itemCounterRecord, &itemEnd };
        Serialization::Item *listFinal[] = { &itemValueFinal, &itemEnd };

        unsigned int localDataLength = 0;
        std::unique_ptr<char[]> localData;

        if( !initCmd( cmd ) ) {
            itemResult.value_char = WRONG_COMMAND;
            localData = Serialization::pack( ( const Serialization::Item **)listNone, localDataLength );

            itemValueFinal.length = localDataLength;
            itemValueFinal.value_string = localData.get();
            return Serialization::pack( ( const Serialization::Item **)listFinal, resultLength );
        }

        if( !initParams( data, length, params ) ) {
            itemResult.value_char = WRONG_PARAMS;
            localData = Serialization::pack( ( const Serialization::Item **)listNone, localDataLength );

            itemValueFinal.length = localDataLength;
            itemValueFinal.value_string = localData.get();
            return Serialization::pack( ( const Serialization::Item **)listFinal, resultLength );
        }

        switch( cmd ) {
            case Commands::GENERATE:
            case Commands::ALL_ADD_KEY:
            case Commands::ALL_REMOVE_KEY:
            case Commands::ALL_SET_LIMIT_PER_SEC_TO_READ:
            case Commands::ALL_SET_LIMIT_PER_SEC_TO_WRITE:
                break;
            default:
                UUID::toNormal( params.uuidRaw, uuid );
                break;
        }

        switch( cmd ) {
            case Commands::GENERATE:
                res = _store->generate( params.lifetime, uuid );
                UUID::toBin( uuid, uuidRaw );
                break;
            case Commands::INIT:
                res = _store->exist( uuid );
                break;
            case Commands::REMOVE:
                _store->remove( uuid );
                break;
            case Commands::PROLONG:
                res = _store->prolong( uuid, params.lifetime );
                break;
            case Commands::ADD_KEY:
                res = _store->addKey(
                    uuid,
                    params.key,
                    params.data,
                    counterKeys,
                    counterRecord,
                    params.lifetime,
                    params.limitWrite,
                    params.limitRead
                );
                break;
            case Commands::ALL_ADD_KEY:
                res = _store->addAllKey(
                    params.key,
                    params.data,
                    params.limitWrite,
                    params.limitRead
                );
                break;
            case Commands::GET_KEY:
                res = _store->getKey( uuid, params.key, value, counterKeys, counterRecord );
                break;
            case Commands::REMOVE_KEY:
                res = _store->removeKey( uuid, params.key );
                break;
            case Commands::ALL_REMOVE_KEY:
                res = _store->removeAllKey( params.key );
                break;
            case Commands::EXIST_KEY:
                res = _store->existKey( uuid, params.key );
                break;
            case Commands::SET_KEY:
                res = _store->setKey(
                    uuid,
                    params.key,
                    params.data,
                    params.dataLength,
                    params.counterKeys,
                    params.counterRecord
                );
                break;
            case Commands::SET_FORCE_KEY:
                res = _store->setForceKey( uuid, params.key, params.data, params.dataLength );
                break;
            case Commands::PROLONG_KEY:
                res = _store->prolongKey( uuid, params.key, params.lifetime );
                break;
            case Commands::SET_LIMIT_PER_SEC_TO_READ:
                res = _store->setLimitToReadPerSec( uuid, params.key, params.limitRead );
                break;
            case Commands::ALL_SET_LIMIT_PER_SEC_TO_READ:
                res = _store->setLimitToReadPerSecAllKey( params.key, params.limitRead );
                break;
            case Commands::SET_LIMIT_PER_SEC_TO_WRITE:
                res = _store->setLimitToWritePerSec( uuid, params.key, params.limitWrite );
                break;
            case Commands::ALL_SET_LIMIT_PER_SEC_TO_WRITE:
                res = _store->setLimitToWritePerSecAllKey( uuid, params.limitWrite );
                break;
        }

        itemResult.value_char = res;


        if( res != StoreInterface::OK ) {
            itemResult.value_char = convertStoreError( res );
            localData = Serialization::pack( ( const Serialization::Item **)listNone, localDataLength );
        } else {
            itemResult.value_char = OK;

            if( cmd == Commands::GENERATE ) {
                itemUUID.value_string = uuidRaw;
                localData = Serialization::pack( ( const Serialization::Item **)listGenerate, localDataLength );
            } else if( cmd == Commands::ADD_KEY ) {
                itemCounterKeys.value_int = counterKeys;
                itemCounterRecord.value_int = counterRecord;
                localData = Serialization::pack( ( const Serialization::Item **)listAddKey, localDataLength );
            } else if( cmd == Commands::GET_KEY ) {
                itemCounterKeys.value_int = counterKeys;
                itemCounterRecord.value_int = counterRecord;
                itemValue.value_string = value.c_str();
                itemValue.length = value.length();

                localData = Serialization::pack( ( const Serialization::Item **)listGetKey, localDataLength );
            } else {
                localData = Serialization::pack( ( const Serialization::Item **)listNone, localDataLength );
            }
        }

        itemValueFinal.length = localDataLength;
        itemValueFinal.value_string = localData.get();
        return Serialization::pack( ( const Serialization::Item **)listFinal, resultLength );
    }

    void ServerController::interval() {
        _store->clearInactive();
    }
}

#endif
