#ifndef MEMSESS_SERVER_CONTROLLER
#define MEMSESS_SERVER_CONTROLLER

#include <arpa/inet.h>
#include <string.h>
#include <string>
#include "../interfaces/server_controller_interface.h"
#include "../interfaces/store_interface.h"
#include "../interfaces/monitoring_interface.h"
#include "../util/uuid.hpp"
#include "../util/serialization.hpp"


namespace memsess::core {
    using namespace util;
    using namespace i;

    class ServerController: public i::ServerControllerInterface {
        private:
            i::StoreInterface *_store;
            i::MonitoringInterface *_monitoring;
            enum Commands {
                GENERATE = 1,
                EXIST = 2,
                REMOVE = 3,
                PROLONG = 4,
                ADD_KEY = 5,
                GET_KEY = 6,
                SET_KEY = 7,
                SET_FORCE_KEY = 8,
                REMOVE_KEY = 9,
                EXIST_KEY = 10,
                PROLONG_KEY = 11,
                ALL_ADD_KEY = 14,
                ALL_REMOVE_KEY = 15,
                ADD_SESSION = 18,
                GET_STATISTICS = 19,
            };
            enum ResultCode {
                OK = 1,
                WRONG_COMMAND = 2,
                WRONG_PARAMS = 3,
                SESSION_NONE = 4,
                KEY_NONE = 5,
                LIMIT_EXCEEDED = 6,
                LIFETIME_EXCEEDED = 7,
                DUPLICATE_KEY = 8,
                RECORD_BEEN_CHANGED = 9,
                LIMIT_PER_SEC_EXCEEDED = 10,
                DUPLICATE_SESSION = 11,
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
            bool isNoUUIDCmd( char cmd );
            void updateMonitoringErrors( ResultCode code );
            void updateMonitoringRequests( unsigned char cmd, ResultCode code );
        public:
            ServerController( i::StoreInterface *store, i::MonitoringInterface *monitoring );
            std::unique_ptr<char[]> parse(
                const char *data,
                unsigned int length,
                unsigned int &resultLength
            );
            void interval();
    };

    ServerController::ServerController( i::StoreInterface *store, i::MonitoringInterface *monitoring ) {
        _store = store;
        _monitoring = monitoring;
    }

    bool ServerController::initCmd( char cmd ) {
        switch( cmd ) {
            case GENERATE:
            case EXIST:
            case REMOVE:
            case PROLONG:
            case ADD_KEY:
            case GET_KEY:
            case SET_KEY:
            case SET_FORCE_KEY:
            case REMOVE_KEY:
            case EXIST_KEY:
            case PROLONG_KEY:
            case ALL_ADD_KEY:
            case ALL_REMOVE_KEY:
            case ADD_SESSION:
            case GET_STATISTICS:
                return true;
            default:
                return false;
        }
    }

    void ServerController::updateMonitoringRequests( unsigned char cmd, ResultCode code ) {
        if( code == ResultCode::OK ) {
            switch( cmd ) {
                case Commands::GENERATE:
                    _monitoring->incPassedGenerate();
                    break;
                case Commands::EXIST:
                    _monitoring->incPassedExist();
                    break;
                case Commands::REMOVE:
                    _monitoring->incPassedRemove();
                    break;
                case Commands::PROLONG:
                    _monitoring->incPassedProlong();
                    break;
                case Commands::ADD_KEY:
                    _monitoring->incPassedAddKey();
                    break;
                case Commands::ALL_ADD_KEY:
                    _monitoring->incPassedAddKeyToAll();
                    break;
                case Commands::GET_KEY:
                    _monitoring->incPassedGetKey();
                    break;
                case Commands::REMOVE_KEY:
                    _monitoring->incPassedRemoveKey();
                    break;
                case Commands::ALL_REMOVE_KEY:
                    _monitoring->incPassedRemoveKeyFromAll();
                    break;
                case Commands::EXIST_KEY:
                    _monitoring->incPassedExistKey();
                    break;
                case Commands::SET_KEY:
                    _monitoring->incPassedSetKey();
                    break;
                case Commands::SET_FORCE_KEY:
                    _monitoring->incPassedSetForceKey();
                    break;
                case Commands::PROLONG_KEY:
                    _monitoring->incPassedProlongKey();
                    break;
                case Commands::ADD_SESSION:
                    _monitoring->incPassedAdd();
                    break;
            }
        } else {
            switch( cmd ) {
                case Commands::GENERATE:
                    _monitoring->incFailedGenerate();
                    break;
                case Commands::EXIST:
                    if( code == SESSION_NONE ) {
                        _monitoring->incPassedExist();
                    } else {
                        _monitoring->incFailedExist();
                    }
                    break;
                case Commands::REMOVE:
                    _monitoring->incFailedRemove();
                    break;
                case Commands::PROLONG:
                    _monitoring->incFailedProlong();
                    break;
                case Commands::ADD_KEY:
                    _monitoring->incFailedAddKey();
                    break;
                case Commands::ALL_ADD_KEY:
                    _monitoring->incFailedAddKeyToAll();
                    break;
                case Commands::GET_KEY:
                    _monitoring->incFailedGetKey();
                    break;
                case Commands::REMOVE_KEY:
                    _monitoring->incFailedRemoveKey();
                    break;
                case Commands::ALL_REMOVE_KEY:
                    _monitoring->incFailedRemoveKeyFromAll();
                    break;
                case Commands::EXIST_KEY:
                    if( code == KEY_NONE ) {
                        _monitoring->incPassedExistKey();
                    } else {
                        _monitoring->incFailedExistKey();
                    }
                    break;
                case Commands::SET_KEY:
                    _monitoring->incFailedSetKey();
                    break;
                case Commands::SET_FORCE_KEY:
                    _monitoring->incFailedSetForceKey();
                    break;
                case Commands::PROLONG_KEY:
                    _monitoring->incFailedProlongKey();
                    break;
                case Commands::ADD_SESSION:
                    _monitoring->incFailedAdd();
                    break;
            }

            updateMonitoringErrors( code );
        }
    }

    void ServerController::updateMonitoringErrors( ResultCode code ) {
        switch( code ) {
            case ResultCode::WRONG_COMMAND:
                _monitoring->incErrorWrongCommand();
                break;
            case ResultCode::WRONG_PARAMS:
                _monitoring->incErrorWrongParams();
                break;
            case ResultCode::SESSION_NONE:
                _monitoring->incErrorSessionNone();
                break;
            case ResultCode::KEY_NONE:
                _monitoring->incErrorKeyNone();
                break;
            case ResultCode::LIMIT_EXCEEDED:
                _monitoring->incErrorLimitExceeded();
                break;
            case ResultCode::LIFETIME_EXCEEDED:
                _monitoring->incErrorLifetimeExceeded();
                break;
            case ResultCode::DUPLICATE_KEY:
                _monitoring->incErrorDuplicateKey();
                break;
            case ResultCode::RECORD_BEEN_CHANGED:
                _monitoring->incErrorRecordBeenChanged();
                break;
            case ResultCode::LIMIT_PER_SEC_EXCEEDED:
                _monitoring->incErrorLimitPerSecExceeded();
                break;
            case ResultCode::DUPLICATE_SESSION:
                _monitoring->incErrorDuplicateSession();
                break;
        }
    }

    bool ServerController::isNoUUIDCmd( char cmd ) {
        switch( cmd ) {
            case Commands::GENERATE:
            case Commands::GET_STATISTICS:
            case Commands::ALL_ADD_KEY:
            case Commands::ALL_REMOVE_KEY:
                return true;
            default:
                return false;
                break;
        }
    }

    ServerController::ResultCode ServerController::convertStoreError( StoreInterface::Result error ) {
        switch( error ) {
            case StoreInterface::E_SESSION_NONE:
                return SESSION_NONE;
            case StoreInterface::E_KEY_NONE:
                return KEY_NONE;
            case StoreInterface::E_LIMIT_EXCEEDED:
                return LIMIT_EXCEEDED;
            case StoreInterface::E_LIFETIME_EXCEEDED:
                return LIFETIME_EXCEEDED;
            case StoreInterface::E_DUPLICATE_KEY:
                return DUPLICATE_KEY;
            case StoreInterface::E_RECORD_BEEN_CHANGED:
                return RECORD_BEEN_CHANGED;
            case StoreInterface::E_LIMIT_PER_SEC_EXCEEDED:
                return LIMIT_PER_SEC_EXCEEDED;
            case StoreInterface::E_DUPLICATE_SESSION:
                return DUPLICATE_SESSION;
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

        Serialization::Item *listGenerate[] = { &lifetime, &end };
        Serialization::Item *listInit[] = { &uuid, &end };
        Serialization::Item *listProlong[] = { &uuid, &lifetime, &end };
        Serialization::Item *listAddKey[] = { &uuid, &key, &value, &lifetime, &end };
        Serialization::Item *listGetKey[] = { &uuid, &key, &limitRead, &end };
        Serialization::Item *listKey[] = { &uuid, &key, &end };
        Serialization::Item *listSetKey[] = { &uuid, &key, &value, &counterKeys, &counterRecord, &limitWrite, &end };
        Serialization::Item *listSetForceKey[] = { &uuid, &key, &value, &limitWrite, &end };
        Serialization::Item *listProlongKey[] = { &uuid, &key, &lifetime, &end };
        Serialization::Item *listAllAddKey[] = { &key, &value, &end };
        Serialization::Item *listAllRemoveKey[] = { &key, &end };
        Serialization::Item *listAddSession[] = { &uuid, &lifetime, &end };
        Serialization::Item *listGetStatistics[] = { &end };

        switch( data[0] ) {
            case Commands::GENERATE:
                if( !Serialization::unpack( listGenerate, &data[1], length - 1 ) ) {
                    return false;
                }

                params.lifetime = ( unsigned int )lifetime.value_int;
                break;
            case Commands::EXIST:
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
                params.lifetime = ( unsigned int )lifetime.value_int;
                break;
            case Commands::ADD_KEY:
                if( !Serialization::unpack( listAddKey, &data[1], length - 1 ) ) {
                    return false;
                }


                params.key = key.value_string;
                params.data = value.value_string;
                params.dataLength = value.length;
                params.uuidRaw = uuid.value_string;
                params.lifetime = ( unsigned int )lifetime.value_int;
                break;
            case Commands::ALL_ADD_KEY:
                if( !Serialization::unpack( listAllAddKey, &data[1], length - 1 ) ) {
                    return false;
                }


                params.key = key.value_string;
                params.data = value.value_string;
                params.dataLength = value.length;
                break;
            case Commands::ALL_REMOVE_KEY:
                if( !Serialization::unpack( listAllRemoveKey, &data[1], length - 1 ) ) {
                    return false;
                }


                params.key = key.value_string;
                break;
            case Commands::GET_KEY:
                if( !Serialization::unpack( listGetKey, &data[1], length - 1 ) ) {
                    return false;
                }

                params.uuidRaw = uuid.value_string;
                params.key = key.value_string;
                params.limitRead = ( unsigned short int )limitRead.value_short_int;

                break;
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
                params.limitWrite = ( unsigned short int )limitWrite.value_short_int;
                break;
            case Commands::SET_FORCE_KEY:
                if( !Serialization::unpack( listSetForceKey, &data[1], length - 1 ) ) {
                    return false;
                }

                params.uuidRaw = uuid.value_string;
                params.key = key.value_string;
                params.data = value.value_string;
                params.dataLength = value.length;
                params.limitWrite = ( unsigned short int )limitWrite.value_short_int;
                break;
            case Commands::PROLONG_KEY:
                if( !Serialization::unpack( listProlongKey, &data[1], length - 1 ) ) {
                    return false;
                }

                params.uuidRaw = uuid.value_string;
                params.key = key.value_string;
                params.lifetime = lifetime.value_int;
                break;
            case Commands::ADD_SESSION:
                if( !Serialization::unpack( listAddSession, &data[1], length - 1 ) ) {
                    return false;
                }

                params.uuidRaw = uuid.value_string;
                params.lifetime = lifetime.value_int;
                break;
            case Commands::GET_STATISTICS:
                if( !Serialization::unpack( listGetStatistics, &data[1], length - 1 ) ) {
                    return false;
                }
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

        i::MonitoringInterface::Data monitoringData;

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




        Serialization::Item itemMonitoringSendedBytes;
        itemMonitoringSendedBytes.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringReceivedBytes;
        itemMonitoringReceivedBytes.type = Serialization::LONG_INT;







        Serialization::Item itemMonitoringPassedGenerate;
        itemMonitoringPassedGenerate.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringPassedExist;
        itemMonitoringPassedExist.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringPassedAdd;
        itemMonitoringPassedAdd.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringPassedProlong;
        itemMonitoringPassedProlong.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringPassedRemove;
        itemMonitoringPassedRemove.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringPassedAddKey;
        itemMonitoringPassedAddKey.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringPassedExistKey;
        itemMonitoringPassedExistKey.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringPassedRemoveKey;
        itemMonitoringPassedRemoveKey.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringPassedProlongKey;
        itemMonitoringPassedProlongKey.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringPassedGetKey;
        itemMonitoringPassedGetKey.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringPassedSetKey;
        itemMonitoringPassedSetKey.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringPassedSetForceKey;
        itemMonitoringPassedSetForceKey.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringPassedAddKeyToAll;
        itemMonitoringPassedAddKeyToAll.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringPassedRemoveKeyFromAll;
        itemMonitoringPassedRemoveKeyFromAll.type = Serialization::LONG_INT;





        Serialization::Item itemMonitoringFailedGenerate;
        itemMonitoringFailedGenerate.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringFailedExist;
        itemMonitoringFailedExist.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringFailedAdd;
        itemMonitoringFailedAdd.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringFailedProlong;
        itemMonitoringFailedProlong.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringFailedRemove;
        itemMonitoringFailedRemove.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringFailedAddKey;
        itemMonitoringFailedAddKey.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringFailedExistKey;
        itemMonitoringFailedExistKey.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringFailedRemoveKey;
        itemMonitoringFailedRemoveKey.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringFailedProlongKey;
        itemMonitoringFailedProlongKey.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringFailedGetKey;
        itemMonitoringFailedGetKey.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringFailedSetKey;
        itemMonitoringFailedSetKey.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringFailedSetForceKey;
        itemMonitoringFailedSetForceKey.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringFailedAddKeyToAll;
        itemMonitoringFailedAddKeyToAll.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringFailedRemoveKeyFromAll;
        itemMonitoringFailedRemoveKeyFromAll.type = Serialization::LONG_INT;




        Serialization::Item itemMonitoringErrorWrongCommand;
        itemMonitoringErrorWrongCommand.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringErrorWrongParams;
        itemMonitoringErrorWrongParams.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringErrorSessionNone;
        itemMonitoringErrorSessionNone.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringErrorKeyNone;
        itemMonitoringErrorKeyNone.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringErrorLimitExceeded;
        itemMonitoringErrorLimitExceeded.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringErrorLifetimeExceeded;
        itemMonitoringErrorLifetimeExceeded.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringErrorDuplicateKey;
        itemMonitoringErrorDuplicateKey.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringErrorRecordBeenChanged;
        itemMonitoringErrorRecordBeenChanged.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringErrorLimitPerSecExceeded;
        itemMonitoringErrorLimitPerSecExceeded.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringErrorDuplicateSession;
        itemMonitoringErrorDuplicateSession.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringErrorDisconnection;
        itemMonitoringErrorDisconnection.type = Serialization::LONG_INT;




        Serialization::Item itemMonitoringDurationReceivingLess5ms;
        itemMonitoringDurationReceivingLess5ms.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringDurationReceivingLess10ms;
        itemMonitoringDurationReceivingLess10ms.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringDurationReceivingLess20ms;
        itemMonitoringDurationReceivingLess20ms.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringDurationReceivingLess50ms;
        itemMonitoringDurationReceivingLess50ms.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringDurationReceivingLess100ms;
        itemMonitoringDurationReceivingLess100ms.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringDurationReceivingLess200ms;
        itemMonitoringDurationReceivingLess200ms.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringDurationReceivingLess500ms;
        itemMonitoringDurationReceivingLess500ms.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringDurationReceivingLess1000ms;
        itemMonitoringDurationReceivingLess1000ms.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringDurationReceivingOther;
        itemMonitoringDurationReceivingOther.type = Serialization::LONG_INT;






        Serialization::Item itemMonitoringDurationProcessingLess5ms;
        itemMonitoringDurationProcessingLess5ms.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringDurationProcessingLess10ms;
        itemMonitoringDurationProcessingLess10ms.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringDurationProcessingLess20ms;
        itemMonitoringDurationProcessingLess20ms.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringDurationProcessingLess50ms;
        itemMonitoringDurationProcessingLess50ms.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringDurationProcessingLess100ms;
        itemMonitoringDurationProcessingLess100ms.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringDurationProcessingLess200ms;
        itemMonitoringDurationProcessingLess200ms.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringDurationProcessingLess500ms;
        itemMonitoringDurationProcessingLess500ms.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringDurationProcessingLess1000ms;
        itemMonitoringDurationProcessingLess1000ms.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringDurationProcessingOther;
        itemMonitoringDurationProcessingOther.type = Serialization::LONG_INT;




        Serialization::Item itemMonitoringDurationSendingLess5ms;
        itemMonitoringDurationSendingLess5ms.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringDurationSendingLess10ms;
        itemMonitoringDurationSendingLess10ms.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringDurationSendingLess20ms;
        itemMonitoringDurationSendingLess20ms.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringDurationSendingLess50ms;
        itemMonitoringDurationSendingLess50ms.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringDurationSendingLess100ms;
        itemMonitoringDurationSendingLess100ms.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringDurationSendingLess200ms;
        itemMonitoringDurationSendingLess200ms.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringDurationSendingLess500ms;
        itemMonitoringDurationSendingLess500ms.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringDurationSendingLess1000ms;
        itemMonitoringDurationSendingLess1000ms.type = Serialization::LONG_INT;

        Serialization::Item itemMonitoringDurationSendingOther;
        itemMonitoringDurationSendingOther.type = Serialization::LONG_INT;


        Serialization::Item itemMonitoringTotalFreeSessions;
        itemMonitoringTotalFreeSessions.type = Serialization::LONG_INT;


        Serialization::Item itemEnd;
        itemEnd.type = Serialization::END;

        Serialization::Item *listNone[] = { &itemResult, &itemEnd };
        Serialization::Item *listGenerate[] = { &itemResult, &itemUUID, &itemEnd };
        Serialization::Item *listAddKey[] = { &itemResult, &itemCounterKeys, &itemCounterRecord, &itemEnd };
        Serialization::Item *listGetKey[] = { &itemResult, &itemValue, &itemCounterKeys, &itemCounterRecord, &itemEnd };
        Serialization::Item *listGetStatics[] = {
            &itemResult,

            &itemMonitoringSendedBytes,
            &itemMonitoringReceivedBytes,

            &itemMonitoringPassedGenerate,
            &itemMonitoringPassedExist,
            &itemMonitoringPassedAdd,
            &itemMonitoringPassedProlong,
            &itemMonitoringPassedRemove,
            &itemMonitoringPassedAddKey,
            &itemMonitoringPassedExistKey,
            &itemMonitoringPassedRemoveKey,
            &itemMonitoringPassedProlongKey,
            &itemMonitoringPassedGetKey,
            &itemMonitoringPassedSetKey,
            &itemMonitoringPassedSetForceKey,
            &itemMonitoringPassedAddKeyToAll,
            &itemMonitoringPassedRemoveKeyFromAll,

            &itemMonitoringFailedGenerate,
            &itemMonitoringFailedExist,
            &itemMonitoringFailedAdd,
            &itemMonitoringFailedProlong,
            &itemMonitoringFailedRemove,
            &itemMonitoringFailedAddKey,
            &itemMonitoringFailedExistKey,
            &itemMonitoringFailedRemoveKey,
            &itemMonitoringFailedProlongKey,
            &itemMonitoringFailedGetKey,
            &itemMonitoringFailedSetKey,
            &itemMonitoringFailedSetForceKey,
            &itemMonitoringFailedAddKeyToAll,
            &itemMonitoringFailedRemoveKeyFromAll,

            &itemMonitoringErrorWrongCommand,
            &itemMonitoringErrorWrongParams,
            &itemMonitoringErrorSessionNone,
            &itemMonitoringErrorKeyNone,
            &itemMonitoringErrorLimitExceeded,
            &itemMonitoringErrorLifetimeExceeded,
            &itemMonitoringErrorDuplicateKey,
            &itemMonitoringErrorRecordBeenChanged,
            &itemMonitoringErrorLimitPerSecExceeded,
            &itemMonitoringErrorDuplicateSession,
            &itemMonitoringErrorDisconnection,

            &itemMonitoringDurationReceivingLess5ms,
            &itemMonitoringDurationReceivingLess10ms,
            &itemMonitoringDurationReceivingLess20ms,
            &itemMonitoringDurationReceivingLess50ms,
            &itemMonitoringDurationReceivingLess100ms,
            &itemMonitoringDurationReceivingLess200ms,
            &itemMonitoringDurationReceivingLess500ms,
            &itemMonitoringDurationReceivingLess1000ms,
            &itemMonitoringDurationReceivingOther,

            &itemMonitoringDurationProcessingLess5ms,
            &itemMonitoringDurationProcessingLess10ms,
            &itemMonitoringDurationProcessingLess20ms,
            &itemMonitoringDurationProcessingLess50ms,
            &itemMonitoringDurationProcessingLess100ms,
            &itemMonitoringDurationProcessingLess200ms,
            &itemMonitoringDurationProcessingLess500ms,
            &itemMonitoringDurationProcessingLess1000ms,
            &itemMonitoringDurationProcessingOther,

            &itemMonitoringDurationSendingLess5ms,
            &itemMonitoringDurationSendingLess10ms,
            &itemMonitoringDurationSendingLess20ms,
            &itemMonitoringDurationSendingLess50ms,
            &itemMonitoringDurationSendingLess100ms,
            &itemMonitoringDurationSendingLess200ms,
            &itemMonitoringDurationSendingLess500ms,
            &itemMonitoringDurationSendingLess1000ms,
            &itemMonitoringDurationSendingOther,

            &itemMonitoringTotalFreeSessions,

            &itemEnd
        };
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

        if( !isNoUUIDCmd( cmd ) ) {
            UUID::toNormal( params.uuidRaw, uuid );
        }

        switch( cmd ) {
            case Commands::GENERATE:
                res = _store->generate( params.lifetime, uuid );
                UUID::toBin( uuid, uuidRaw );
                break;
            case Commands::EXIST:
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
                    params.dataLength,
                    counterKeys,
                    counterRecord,
                    params.lifetime
                );
                break;
            case Commands::ALL_ADD_KEY:
                res = _store->addAllKey(
                    params.key,
                    params.data,
                    params.dataLength
                );
                break;
            case Commands::GET_KEY:
                res = _store->getKey( uuid, params.key, value, counterKeys, counterRecord, params.limitRead );
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
                    params.counterRecord,
                    params.limitWrite
                );
                break;
            case Commands::SET_FORCE_KEY:
                res = _store->setForceKey( uuid, params.key, params.data, params.dataLength, params.limitWrite );
                break;
            case Commands::PROLONG_KEY:
                res = _store->prolongKey( uuid, params.key, params.lifetime );
                break;
            case Commands::ADD_SESSION:
                res = _store->add( uuid, params.lifetime );
                break;
            case Commands::GET_STATISTICS:
                _monitoring->getData( monitoringData );
                break;
        }

        itemResult.value_char = res;
        auto error = convertStoreError( res );
        updateMonitoringRequests( cmd, error );

        if( res != StoreInterface::OK ) {
            itemResult.value_char = error;
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
            } else if( cmd == Commands::GET_STATISTICS ) {
                itemMonitoringSendedBytes.value_long_int = monitoringData.traffic.sendedBytes;
                itemMonitoringReceivedBytes.value_long_int = monitoringData.traffic.receivedBytes;

                itemMonitoringPassedGenerate.value_long_int = monitoringData.passedRequests.generate;
                itemMonitoringPassedExist.value_long_int = monitoringData.passedRequests.exist;
                itemMonitoringPassedAdd.value_long_int = monitoringData.passedRequests.add;
                itemMonitoringPassedProlong.value_long_int = monitoringData.passedRequests.prolong;
                itemMonitoringPassedRemove.value_long_int = monitoringData.passedRequests.remove;
                itemMonitoringPassedAddKey.value_long_int = monitoringData.passedRequests.addKey;
                itemMonitoringPassedExistKey.value_long_int = monitoringData.passedRequests.existKey;
                itemMonitoringPassedRemoveKey.value_long_int = monitoringData.passedRequests.removeKey;
                itemMonitoringPassedProlongKey.value_long_int = monitoringData.passedRequests.prolongKey;
                itemMonitoringPassedGetKey.value_long_int = monitoringData.passedRequests.getKey;
                itemMonitoringPassedSetKey.value_long_int = monitoringData.passedRequests.setKey;
                itemMonitoringPassedSetForceKey.value_long_int = monitoringData.passedRequests.setForceKey;
                itemMonitoringPassedAddKeyToAll.value_long_int = monitoringData.passedRequests.addKeyToAll;
                itemMonitoringPassedRemoveKeyFromAll.value_long_int = monitoringData.passedRequests.removeKeyFromAll;

                itemMonitoringFailedGenerate.value_long_int = monitoringData.failedRequests.generate;
                itemMonitoringFailedExist.value_long_int = monitoringData.failedRequests.exist;
                itemMonitoringFailedAdd.value_long_int = monitoringData.failedRequests.add;
                itemMonitoringFailedProlong.value_long_int = monitoringData.failedRequests.prolong;
                itemMonitoringFailedRemove.value_long_int = monitoringData.failedRequests.remove;
                itemMonitoringFailedAddKey.value_long_int = monitoringData.failedRequests.addKey;
                itemMonitoringFailedExistKey.value_long_int = monitoringData.failedRequests.existKey;
                itemMonitoringFailedRemoveKey.value_long_int = monitoringData.failedRequests.removeKey;
                itemMonitoringFailedProlongKey.value_long_int = monitoringData.failedRequests.prolongKey;
                itemMonitoringFailedGetKey.value_long_int = monitoringData.failedRequests.getKey;
                itemMonitoringFailedSetKey.value_long_int = monitoringData.failedRequests.setKey;
                itemMonitoringFailedSetForceKey.value_long_int = monitoringData.failedRequests.setForceKey;
                itemMonitoringFailedAddKeyToAll.value_long_int = monitoringData.failedRequests.addKeyToAll;
                itemMonitoringFailedRemoveKeyFromAll.value_long_int = monitoringData.failedRequests.removeKeyFromAll;

                itemMonitoringErrorWrongCommand.value_long_int = monitoringData.errors.wrongCommand;
                itemMonitoringErrorWrongParams.value_long_int = monitoringData.errors.wrongParams;
                itemMonitoringErrorSessionNone.value_long_int = monitoringData.errors.sessionNone;
                itemMonitoringErrorKeyNone.value_long_int = monitoringData.errors.keyNone;
                itemMonitoringErrorLimitExceeded.value_long_int = monitoringData.errors.limitExceeded;
                itemMonitoringErrorLifetimeExceeded.value_long_int = monitoringData.errors.lifetimeExceeded;
                itemMonitoringErrorDuplicateKey.value_long_int = monitoringData.errors.duplicateKey;
                itemMonitoringErrorRecordBeenChanged.value_long_int = monitoringData.errors.recordBeenChanged;
                itemMonitoringErrorLimitPerSecExceeded.value_long_int = monitoringData.errors.limitPerSecExceeded;
                itemMonitoringErrorDuplicateSession.value_long_int = monitoringData.errors.duplicateSession;
                itemMonitoringErrorDisconnection.value_long_int = monitoringData.errors.disconnection;

                itemMonitoringDurationReceivingLess5ms.value_long_int = monitoringData.durationReceiving.less5ms;
                itemMonitoringDurationReceivingLess10ms.value_long_int = monitoringData.durationReceiving.less10ms;
                itemMonitoringDurationReceivingLess20ms.value_long_int = monitoringData.durationReceiving.less20ms;
                itemMonitoringDurationReceivingLess50ms.value_long_int = monitoringData.durationReceiving.less50ms;
                itemMonitoringDurationReceivingLess100ms.value_long_int = monitoringData.durationReceiving.less100ms;
                itemMonitoringDurationReceivingLess200ms.value_long_int = monitoringData.durationReceiving.less200ms;
                itemMonitoringDurationReceivingLess500ms.value_long_int = monitoringData.durationReceiving.less500ms;
                itemMonitoringDurationReceivingLess1000ms.value_long_int = monitoringData.durationReceiving.less1000ms;
                itemMonitoringDurationReceivingOther.value_long_int = monitoringData.durationReceiving.other;


                itemMonitoringDurationProcessingLess5ms.value_long_int = monitoringData.durationProcessing.less5ms;
                itemMonitoringDurationProcessingLess10ms.value_long_int = monitoringData.durationProcessing.less10ms;
                itemMonitoringDurationProcessingLess20ms.value_long_int = monitoringData.durationProcessing.less20ms;
                itemMonitoringDurationProcessingLess50ms.value_long_int = monitoringData.durationProcessing.less50ms;
                itemMonitoringDurationProcessingLess100ms.value_long_int = monitoringData.durationProcessing.less100ms;
                itemMonitoringDurationProcessingLess200ms.value_long_int = monitoringData.durationProcessing.less200ms;
                itemMonitoringDurationProcessingLess500ms.value_long_int = monitoringData.durationProcessing.less500ms;
                itemMonitoringDurationProcessingLess1000ms.value_long_int = monitoringData.durationProcessing.less1000ms;
                itemMonitoringDurationProcessingOther.value_long_int = monitoringData.durationProcessing.other;

                itemMonitoringDurationSendingLess5ms.value_long_int = monitoringData.durationSending.less5ms;
                itemMonitoringDurationSendingLess10ms.value_long_int = monitoringData.durationSending.less10ms;
                itemMonitoringDurationSendingLess20ms.value_long_int = monitoringData.durationSending.less20ms;
                itemMonitoringDurationSendingLess50ms.value_long_int = monitoringData.durationSending.less50ms;
                itemMonitoringDurationSendingLess100ms.value_long_int = monitoringData.durationSending.less100ms;
                itemMonitoringDurationSendingLess200ms.value_long_int = monitoringData.durationSending.less200ms;
                itemMonitoringDurationSendingLess500ms.value_long_int = monitoringData.durationSending.less500ms;
                itemMonitoringDurationSendingLess1000ms.value_long_int = monitoringData.durationSending.less1000ms;
                itemMonitoringDurationSendingOther.value_long_int = monitoringData.durationSending.other;

                itemMonitoringTotalFreeSessions.value_long_int = monitoringData.totalFreeSessions;




                localData = Serialization::pack( ( const Serialization::Item **)listGetStatics, localDataLength );
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
