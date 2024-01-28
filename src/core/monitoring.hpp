#ifndef MEMSESS_CORE_MONITORING
#define MEMSESS_CORE_MONITORING

#include "../interfaces/monitoring_interface.h"
#include <atomic>

namespace memsess::core {
    class Monitoring: public i::MonitoringInterface {
        private:
            std::atomic<unsigned long int> _sendedBytes{ 0 };
            std::atomic<unsigned long int> _receivedBytes{ 0 };

            std::atomic<unsigned long int> _passedGenerate{ 0 };
            std::atomic<unsigned long int> _passedExist{ 0 };
            std::atomic<unsigned long int> _passedAdd{ 0 };
            std::atomic<unsigned long int> _passedProlong{ 0 };
            std::atomic<unsigned long int> _passedRemove{ 0 };
            std::atomic<unsigned long int> _passedAddKey{ 0 };
            std::atomic<unsigned long int> _passedExistKey{ 0 };
            std::atomic<unsigned long int> _passedRemoveKey{ 0 };
            std::atomic<unsigned long int> _passedProlongKey{ 0 };
            std::atomic<unsigned long int> _passedGetKey{ 0 };
            std::atomic<unsigned long int> _passedSetKey{ 0 };
            std::atomic<unsigned long int> _passedSetForceKey{ 0 };
            std::atomic<unsigned long int> _passedAddKeyToAll{ 0 };
            std::atomic<unsigned long int> _passedRemoveKeyFromAll{ 0 };

            std::atomic<unsigned long int> _failedGenerate{ 0 };
            std::atomic<unsigned long int> _failedExist{ 0 };
            std::atomic<unsigned long int> _failedAdd{ 0 };
            std::atomic<unsigned long int> _failedProlong{ 0 };
            std::atomic<unsigned long int> _failedRemove{ 0 };
            std::atomic<unsigned long int> _failedAddKey{ 0 };
            std::atomic<unsigned long int> _failedExistKey{ 0 };
            std::atomic<unsigned long int> _failedRemoveKey{ 0 };
            std::atomic<unsigned long int> _failedProlongKey{ 0 };
            std::atomic<unsigned long int> _failedGetKey{ 0 };
            std::atomic<unsigned long int> _failedSetKey{ 0 };
            std::atomic<unsigned long int> _failedSetForceKey{ 0 };
            std::atomic<unsigned long int> _failedAddKeyToAll{ 0 };
            std::atomic<unsigned long int> _failedRemoveKeyFromAll{ 0 };

            std::atomic<unsigned long int> _errorWrongCommand{ 0 };
            std::atomic<unsigned long int> _errorWrongParams{ 0 };
            std::atomic<unsigned long int> _errorSessionNone{ 0 };
            std::atomic<unsigned long int> _errorKeyNone{ 0 };
            std::atomic<unsigned long int> _errorLimitExceeded{ 0 };
            std::atomic<unsigned long int> _errorLifetimeExceeded{ 0 };
            std::atomic<unsigned long int> _errorDuplicateKey{ 0 };
            std::atomic<unsigned long int> _errorRecordBeenChanged{ 0 };
            std::atomic<unsigned long int> _errorLimitPerSecExceeded{ 0 };
            std::atomic<unsigned long int> _errorDuplicateSession{ 0 };
            std::atomic<unsigned long int> _errorDisconnection{ 0 };

            std::atomic<unsigned long int> _durationReceivingLess5ms{ 0 };
            std::atomic<unsigned long int> _durationReceivingLess10ms{ 0 };
            std::atomic<unsigned long int> _durationReceivingLess20ms{ 0 };
            std::atomic<unsigned long int> _durationReceivingLess50ms{ 0 };
            std::atomic<unsigned long int> _durationReceivingLess100ms{ 0 };
            std::atomic<unsigned long int> _durationReceivingLess200ms{ 0 };
            std::atomic<unsigned long int> _durationReceivingLess500ms{ 0 };
            std::atomic<unsigned long int> _durationReceivingLess1000ms{ 0 };
            std::atomic<unsigned long int> _durationReceivingOther{ 0 };

            std::atomic<unsigned long int> _durationSendingLess5ms{ 0 };
            std::atomic<unsigned long int> _durationSendingLess10ms{ 0 };
            std::atomic<unsigned long int> _durationSendingLess20ms{ 0 };
            std::atomic<unsigned long int> _durationSendingLess50ms{ 0 };
            std::atomic<unsigned long int> _durationSendingLess100ms{ 0 };
            std::atomic<unsigned long int> _durationSendingLess200ms{ 0 };
            std::atomic<unsigned long int> _durationSendingLess500ms{ 0 };
            std::atomic<unsigned long int> _durationSendingLess1000ms{ 0 };
            std::atomic<unsigned long int> _durationSendingOther{ 0 };

            std::atomic<unsigned long int> _durationProcessingLess5ms{ 0 };
            std::atomic<unsigned long int> _durationProcessingLess10ms{ 0 };
            std::atomic<unsigned long int> _durationProcessingLess20ms{ 0 };
            std::atomic<unsigned long int> _durationProcessingLess50ms{ 0 };
            std::atomic<unsigned long int> _durationProcessingLess100ms{ 0 };
            std::atomic<unsigned long int> _durationProcessingLess200ms{ 0 };
            std::atomic<unsigned long int> _durationProcessingLess500ms{ 0 };
            std::atomic<unsigned long int> _durationProcessingLess1000ms{ 0 };
            std::atomic<unsigned long int> _durationProcessingOther{ 0 };

            std::atomic<unsigned int> _totalFreeSessions{ 0 };

        public:
            void incSendedBytes( unsigned int );
            void incReceivedBytes( unsigned int );

            void incPassedGenerate();
            void incPassedExist();
            void incPassedAdd();
            void incPassedProlong();
            void incPassedRemove();
            void incPassedAddKey();
            void incPassedExistKey();
            void incPassedRemoveKey();
            void incPassedProlongKey();
            void incPassedGetKey();
            void incPassedSetKey();
            void incPassedSetForceKey();
            void incPassedAddKeyToAll();
            void incPassedRemoveKeyFromAll();

            void incFailedGenerate();
            void incFailedExist();
            void incFailedAdd();
            void incFailedProlong();
            void incFailedRemove();
            void incFailedAddKey();
            void incFailedExistKey();
            void incFailedRemoveKey();
            void incFailedProlongKey();
            void incFailedGetKey();
            void incFailedSetKey();
            void incFailedSetForceKey();
            void incFailedAddKeyToAll();
            void incFailedRemoveKeyFromAll();

            void incErrorWrongCommand();
            void incErrorWrongParams();
            void incErrorSessionNone();
            void incErrorKeyNone();
            void incErrorLimitExceeded();
            void incErrorLifetimeExceeded();
            void incErrorDuplicateKey();
            void incErrorRecordBeenChanged();
            void incErrorLimitPerSecExceeded();
            void incErrorDuplicateSession();
            void incErrorDisconnection();

            void updateDurationReceiving( unsigned int );
            void updateDurationProcessing( unsigned int );
            void updateDurationSending( unsigned int );

            void updateTotalFreeSessions( unsigned int );

            void getData( Data &data );
    };

    void Monitoring::incSendedBytes( unsigned int bytes ) {
        _sendedBytes += bytes;
    }

    void Monitoring::incReceivedBytes( unsigned int bytes ) {
        _receivedBytes += bytes;
    }

    void Monitoring::incPassedGenerate() {
        _passedGenerate++;
    }

    void Monitoring::incPassedExist() {
        _passedExist++;
    }

    void Monitoring::incPassedAdd() {
        _passedAdd++;
    }

    void Monitoring::incPassedProlong() {
        _passedProlong++;
    }

    void Monitoring::incPassedRemove() {
        _passedRemove++;
    }

    void Monitoring::incPassedAddKey() {
        _passedAddKey++;
    }

    void Monitoring::incPassedExistKey() {
        _passedExistKey++;
    }

    void Monitoring::incPassedRemoveKey() {
        _passedRemoveKey++;
    }

    void Monitoring::incPassedProlongKey() {
        _passedProlongKey++;
    }

    void Monitoring::incPassedGetKey() {
        _passedGetKey++;
    }

    void Monitoring::incPassedSetKey() {
        _passedSetKey++;
    }

    void Monitoring::incPassedSetForceKey() {
        _passedSetForceKey++;
    }

    void Monitoring::incPassedAddKeyToAll() {
        _passedAddKeyToAll++;
    }

    void Monitoring::incPassedRemoveKeyFromAll() {
        _passedRemoveKeyFromAll++;
    }


    void Monitoring::incFailedGenerate() {
        _failedGenerate++;
    }

    void Monitoring::incFailedExist() {
        _failedExist++;
    }

    void Monitoring::incFailedAdd() {
        _failedAdd++;
    }

    void Monitoring::incFailedProlong() {
        _failedProlong++;
    }

    void Monitoring::incFailedRemove() {
        _failedRemove++;
    }

    void Monitoring::incFailedAddKey() {
        _failedAddKey++;
    }

    void Monitoring::incFailedExistKey() {
        _failedExistKey++;
    }

    void Monitoring::incFailedRemoveKey() {
        _failedRemoveKey++;
    }

    void Monitoring::incFailedProlongKey() {
        _failedProlongKey++;
    }

    void Monitoring::incFailedGetKey() {
        _failedGetKey++;
    }

    void Monitoring::incFailedSetKey() {
        _failedSetKey++;
    }

    void Monitoring::incFailedSetForceKey() {
        _failedSetForceKey++;
    }

    void Monitoring::incFailedAddKeyToAll() {
        _failedAddKeyToAll++;
    }

    void Monitoring::incFailedRemoveKeyFromAll() {
        _failedRemoveKeyFromAll++;
    }

    void Monitoring::incErrorWrongCommand() {
        _errorWrongCommand++;
    }

    void Monitoring::incErrorWrongParams() {
        _errorWrongParams++;
    }

    void Monitoring::incErrorSessionNone() {
        _errorSessionNone++;
    }

    void Monitoring::incErrorKeyNone() {
        _errorKeyNone++;
    }

    void Monitoring::incErrorLimitExceeded() {
        _errorLimitExceeded++;
    }

    void Monitoring::incErrorLifetimeExceeded() {
        _errorLifetimeExceeded++;
    }

    void Monitoring::incErrorDuplicateKey() {
        _errorDuplicateKey++;
    }

    void Monitoring::incErrorRecordBeenChanged() {
        _errorRecordBeenChanged++;
    }

    void Monitoring::incErrorLimitPerSecExceeded() {
        _errorLimitPerSecExceeded++;
    }

    void Monitoring::incErrorDuplicateSession() {
        _errorDuplicateSession++;
    }

    void Monitoring::incErrorDisconnection() {
        _errorDisconnection++;
    }

    void Monitoring::updateDurationReceiving( unsigned int ms ) {
        if( ms < 5 ) {
            _durationReceivingLess5ms++;
        } else if( ms < 10 ) {
            _durationReceivingLess10ms++;
        } else if( ms < 20 ) {
            _durationReceivingLess20ms++;
        } else if( ms < 50 ) {
            _durationReceivingLess50ms++;
        } else if( ms < 100 ) {
            _durationReceivingLess100ms++;
        } else if( ms < 200 ) {
            _durationReceivingLess200ms++;
        } else if( ms < 500 ) {
            _durationReceivingLess500ms++;
        } else if( ms < 1000 ) {
            _durationReceivingLess1000ms++;
        } else {
            _durationReceivingOther++;
        }
    }

    void Monitoring::updateDurationProcessing( unsigned int ms ) {
        if( ms < 5 ) {
            _durationProcessingLess5ms++;
        } else if( ms < 10 ) {
            _durationProcessingLess10ms++;
        } else if( ms < 20 ) {
            _durationProcessingLess20ms++;
        } else if( ms < 50 ) {
            _durationProcessingLess50ms++;
        } else if( ms < 100 ) {
            _durationProcessingLess100ms++;
        } else if( ms < 200 ) {
            _durationProcessingLess200ms++;
        } else if( ms < 500 ) {
            _durationProcessingLess500ms++;
        } else if( ms < 1000 ) {
            _durationProcessingLess1000ms++;
        } else {
            _durationProcessingOther++;
        }
    }

    void Monitoring::updateDurationSending( unsigned int ms ) {
        if( ms < 5 ) {
            _durationSendingLess5ms++;
        } else if( ms < 10 ) {
            _durationSendingLess10ms++;
        } else if( ms < 20 ) {
            _durationSendingLess20ms++;
        } else if( ms < 50 ) {
            _durationSendingLess50ms++;
        } else if( ms < 100 ) {
            _durationSendingLess100ms++;
        } else if( ms < 200 ) {
            _durationSendingLess200ms++;
        } else if( ms < 500 ) {
            _durationSendingLess500ms++;
        } else if( ms < 1000 ) {
            _durationSendingLess1000ms++;
        } else {
            _durationSendingOther++;
        }
    }

    void Monitoring::updateTotalFreeSessions( unsigned int total ) {
        _totalFreeSessions = total;
    }

    void Monitoring::getData( Data &data ) {
        data.traffic.sendedBytes = _sendedBytes;
        data.traffic.receivedBytes = _receivedBytes;

        data.passedRequests.generate = _passedGenerate;
        data.passedRequests.exist = _passedExist;
        data.passedRequests.add = _passedAdd;
        data.passedRequests.prolong = _passedProlong;
        data.passedRequests.remove = _passedRemove;
        data.passedRequests.addKey = _passedAddKey;
        data.passedRequests.existKey = _passedExistKey;
        data.passedRequests.removeKey = _passedRemoveKey;
        data.passedRequests.prolongKey = _passedProlongKey;
        data.passedRequests.getKey = _passedGetKey;
        data.passedRequests.setKey = _passedSetKey;
        data.passedRequests.setForceKey = _passedSetForceKey;
        data.passedRequests.addKeyToAll = _passedAddKeyToAll;
        data.passedRequests.removeKeyFromAll = _passedRemoveKeyFromAll;

        data.failedRequests.generate = _failedGenerate;
        data.failedRequests.exist = _failedExist;
        data.failedRequests.add = _failedAdd;
        data.failedRequests.prolong = _failedProlong;
        data.failedRequests.remove = _failedRemove;
        data.failedRequests.addKey = _failedAddKey;
        data.failedRequests.existKey = _failedExistKey;
        data.failedRequests.removeKey = _failedRemoveKey;
        data.failedRequests.prolongKey = _failedProlongKey;
        data.failedRequests.getKey = _failedGetKey;
        data.failedRequests.setKey = _failedSetKey;
        data.failedRequests.setForceKey = _failedSetForceKey;
        data.failedRequests.addKeyToAll = _failedAddKeyToAll;
        data.failedRequests.removeKeyFromAll = _failedRemoveKeyFromAll;

        data.errors.wrongCommand = _errorWrongCommand;
        data.errors.wrongParams = _errorWrongParams;
        data.errors.sessionNone = _errorSessionNone;
        data.errors.keyNone = _errorKeyNone;
        data.errors.limitExceeded = _errorLimitExceeded;
        data.errors.lifetimeExceeded = _errorLifetimeExceeded;
        data.errors.duplicateKey = _errorDuplicateKey;
        data.errors.recordBeenChanged = _errorRecordBeenChanged;
        data.errors.limitPerSecExceeded = _errorLimitPerSecExceeded;
        data.errors.duplicateSession = _errorDuplicateSession;
        data.errors.disconnection = _errorDisconnection;

        data.durationReceiving.less5ms = _durationReceivingLess5ms;
        data.durationReceiving.less10ms = _durationReceivingLess10ms;
        data.durationReceiving.less20ms = _durationReceivingLess20ms;
        data.durationReceiving.less50ms = _durationReceivingLess50ms;
        data.durationReceiving.less100ms = _durationReceivingLess100ms;
        data.durationReceiving.less200ms = _durationReceivingLess200ms;
        data.durationReceiving.less500ms = _durationReceivingLess500ms;
        data.durationReceiving.less1000ms = _durationReceivingLess1000ms;
        data.durationReceiving.other = _durationReceivingOther;

        data.durationProcessing.less5ms = _durationProcessingLess5ms;
        data.durationProcessing.less10ms = _durationProcessingLess10ms;
        data.durationProcessing.less20ms = _durationProcessingLess20ms;
        data.durationProcessing.less50ms = _durationProcessingLess50ms;
        data.durationProcessing.less100ms = _durationProcessingLess100ms;
        data.durationProcessing.less200ms = _durationProcessingLess200ms;
        data.durationProcessing.less500ms = _durationProcessingLess500ms;
        data.durationProcessing.less1000ms = _durationProcessingLess1000ms;
        data.durationProcessing.other = _durationProcessingOther;

        data.durationSending.less5ms = _durationSendingLess5ms;
        data.durationSending.less10ms = _durationSendingLess10ms;
        data.durationSending.less20ms = _durationSendingLess20ms;
        data.durationSending.less50ms = _durationSendingLess50ms;
        data.durationSending.less100ms = _durationSendingLess100ms;
        data.durationSending.less200ms = _durationSendingLess200ms;
        data.durationSending.less500ms = _durationSendingLess500ms;
        data.durationSending.less1000ms = _durationSendingLess1000ms;
        data.durationSending.other = _durationSendingOther;

        data.totalFreeSessions = _totalFreeSessions;
    }
}

#endif
