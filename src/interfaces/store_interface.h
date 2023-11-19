#ifndef MEMSESS_I_STORE
#define MEMSESS_I_STORE

#include <string>

namespace memsess::i {
    class StoreInterface {
        public:
            enum Result {
                OK,
                E_SESSION_NONE,
                E_KEY_NONE,
                E_LIMIT,
                E_LIFETIME,
                E_DUPLICATE_KEY,
                E_RECORD_BEEN_CHANGED,
                E_LIMIT_PER_SEC,
            };
            virtual void setLimit( unsigned int limit ) = 0;

            virtual Result generate( unsigned int lifetime, char *sessionId ) = 0;
            virtual Result exist( const char *sessionId ) = 0;
            virtual void remove( const char *sessionId ) = 0;
            virtual Result prolong( const char *sessionId, unsigned int lifetime ) = 0;
         
            virtual Result addKey(
                const char *sessionId,
                const char *key,
                const char *value,
                unsigned int &counterKeys,
                unsigned int &counterRecord,
                unsigned int lifetime = 0,
                unsigned short int limitWrite = 0,
                unsigned short int limitRead = 0
            ) = 0;
            virtual Result existKey( const char *sessionId, const char *key ) = 0;
            virtual Result prolongKey(
                const char *sessionId,
                const char *key,
                unsigned int lifetime
            ) = 0;
            virtual Result setKey(
                const char *sessionId,
                const char *key,
                const char *value,
                unsigned int counterKeys,
                unsigned int counterRecord
            ) = 0;
            virtual Result setForceKey(
                const char *sessionId,
                const char *key,
                const char *value
            ) = 0;
            virtual Result getKey(
                const char *sessionId,
                const char *key,
                std::string &value,
                unsigned int &counterKeys,
                unsigned int &counterRecord
            ) = 0;
            virtual Result removeKey( const char *sessionId, const char *key ) = 0;
            virtual Result setLimitToReadPerSec( const char *sessionId, const char *key, unsigned int limit ) = 0;
            virtual Result setLimitToWritePerSec( const char *sessionId, const char *key, unsigned int limit ) = 0;
         
            virtual void clearInactive() = 0;
    };
}

#endif
