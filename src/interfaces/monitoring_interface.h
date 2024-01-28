#ifndef MEMSESS_I_MONITORING
#define MEMSESS_I_MONITORING

#include <string>

namespace memsess::i {
    class MonitoringInterface {
        public:
            struct DataTraffic {
                unsigned long int sendedBytes;
                unsigned long int receivedBytes;
            };

            struct DataDuration {
                unsigned long int less5ms;
                unsigned long int less10ms;
                unsigned long int less20ms;
                unsigned long int less50ms;
                unsigned long int less100ms;
                unsigned long int less200ms;
                unsigned long int less500ms;
                unsigned long int less1000ms;
                unsigned long int other;
            };

            struct DataMethods {
                unsigned long int generate;
                unsigned long int exist;
                unsigned long int add;
                unsigned long int prolong;
                unsigned long int remove;
                unsigned long int addKey;
                unsigned long int existKey;
                unsigned long int removeKey;
                unsigned long int prolongKey;
                unsigned long int getKey;
                unsigned long int setKey;
                unsigned long int setForceKey;
                unsigned long int addKeyToAll;
                unsigned long int removeKeyFromAll;
            };

            struct DataErrors {
                unsigned long int wrongCommand;
                unsigned long int wrongParams;
                unsigned long int sessionNone;
                unsigned long int keyNone;
                unsigned long int limitExceeded;
                unsigned long int lifetimeExceeded;
                unsigned long int duplicateKey;
                unsigned long int recordBeenChanged;
                unsigned long int limitPerSecExceeded;
                unsigned long int duplicateSession;
                unsigned long int disconnection;
            };

            struct Data {
                DataTraffic traffic;
                DataMethods passedRequests;
                DataMethods failedRequests;
                DataErrors errors;
                DataDuration durationReceiving;
                DataDuration durationProcessing;
                DataDuration durationSending;
                unsigned long int totalFreeSessions;
            };

            virtual void incSendedBytes( unsigned int ) = 0;
            virtual void incReceivedBytes( unsigned int ) = 0;

            virtual void incPassedGenerate() = 0;
            virtual void incPassedExist() = 0;
            virtual void incPassedAdd() = 0;
            virtual void incPassedProlong() = 0;
            virtual void incPassedRemove() = 0;
            virtual void incPassedAddKey() = 0;
            virtual void incPassedExistKey() = 0;
            virtual void incPassedRemoveKey() = 0;
            virtual void incPassedProlongKey() = 0;
            virtual void incPassedGetKey() = 0;
            virtual void incPassedSetKey() = 0;
            virtual void incPassedSetForceKey() = 0;
            virtual void incPassedAddKeyToAll() = 0;
            virtual void incPassedRemoveKeyFromAll() = 0;

            virtual void incFailedGenerate() = 0;
            virtual void incFailedExist() = 0;
            virtual void incFailedAdd() = 0;
            virtual void incFailedProlong() = 0;
            virtual void incFailedRemove() = 0;
            virtual void incFailedAddKey() = 0;
            virtual void incFailedExistKey() = 0;
            virtual void incFailedRemoveKey() = 0;
            virtual void incFailedProlongKey() = 0;
            virtual void incFailedGetKey() = 0;
            virtual void incFailedSetKey() = 0;
            virtual void incFailedSetForceKey() = 0;
            virtual void incFailedAddKeyToAll() = 0;
            virtual void incFailedRemoveKeyFromAll() = 0;

            virtual void incErrorWrongCommand() = 0;
            virtual void incErrorWrongParams() = 0;
            virtual void incErrorSessionNone() = 0;
            virtual void incErrorKeyNone() = 0;
            virtual void incErrorLimitExceeded() = 0;
            virtual void incErrorLifetimeExceeded() = 0;
            virtual void incErrorDuplicateKey() = 0;
            virtual void incErrorRecordBeenChanged() = 0;
            virtual void incErrorLimitPerSecExceeded() = 0;
            virtual void incErrorDuplicateSession() = 0;
            virtual void incErrorDisconnection() = 0;

            virtual void updateDurationReceiving( unsigned int ) = 0;
            virtual void updateDurationProcessing( unsigned int ) = 0;
            virtual void updateDurationSending( unsigned int ) = 0;

            virtual void updateTotalFreeSessions( unsigned int ) = 0;

            virtual void getData( Data &data ) = 0;

    };
}

#endif
