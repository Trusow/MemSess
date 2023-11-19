#ifndef MEMSESS_CORE_STORE
#define MEMSESS_CORE_STORE

#include <unordered_map>
#include <shared_mutex>
#include <string>
#include <string.h>
#include <time.h>
#include "../interfaces/store_interface.h"
#include "../util/uuid.hpp"

namespace memsess::core {
    class Store: public i::StoreInterface {

        public:

            struct Limiter {
                unsigned long int ts; 
                unsigned int limit;
                unsigned int count;
            };
 
            struct Value {
                std::string value;
                unsigned long int tsEnd;
                unsigned int counterRecord;
                std::unique_ptr<Limiter> limiterRead;
                std::unique_ptr<Limiter> limiterWrite;
            };
 
            struct Item {
                std::unordered_map<std::string, std::unique_ptr<Value>> values;
                unsigned int counterKeys;
                unsigned long int tsEnd;
            };
 
        private:
            std::unordered_map<std::string, std::unique_ptr<Item>> _list;
            unsigned int _limit;
            unsigned int _count = 0;
            unsigned long int getTime();
            bool incLimiter( Limiter *limiter );
 
        public:
            Result generate( unsigned int lifetime, char *sessionId );
            Result exist( const char *sessionId );
            void setLimit( unsigned int limit );
            void remove( const char *sessionId );
            Result prolong( const char *sessionId, unsigned int lifetime );
         
            Result addKey(
                const char *sessionId,
                const char *key,
                const char *value,
                unsigned int &counterKeys,
                unsigned int &counterRecord,
                unsigned int lifetime = 0,
                unsigned short int limitWrite = 0,
                unsigned short int limitRead = 0
            );
            Result existKey( const char *sessionId, const char *key );
            Result prolongKey( const char *sessionId, const char *key, unsigned int lifetime );
            Result setKey(
                const char *sessionId,
                const char *key,
                const char *value,
                unsigned int length,
                unsigned int counterKeys,
                unsigned int counterRecord
            );
            Result setForceKey(
                const char *sessionId,
                const char *key,
                const char *value,
                unsigned int length
            );
            Result getKey( const char *sessionId, const char *key, std::string &value, unsigned int &counterKeys, unsigned int &counterRecord );
            Result removeKey( const char *sessionId, const char *key );
            Result setLimitToReadPerSec( const char *sessionId, const char *key, unsigned int limit );
            Result setLimitToWritePerSec( const char *sessionId, const char *key, unsigned int limit );
         
            void clearInactive();
    };

    unsigned long int Store::getTime() {
        return time( NULL );
    }

    Store::Result Store::generate( unsigned int lifetime, char *sessionId ) {
        if( ( _count == _limit && _limit != 0 ) || _count == 0xFF'FF'FF'FF ) {
            return Result::E_LIMIT;
        }

        if( lifetime == 0 ) {
            return Result::E_LIFETIME;
        }

        while( true ) {
            util::UUID::generate( sessionId );

            if( _list.find( sessionId ) == _list.end() ) {
                break;
            }
        }

        auto item = std::make_unique<Item>();
        item->tsEnd = getTime() + lifetime;

        _list[sessionId] = std::move( item );
        _count++;

        return Result::OK;
    }

    Store::Result Store::exist( const char *sessionId ) {
        if( _list.find( sessionId ) != _list.end() ) {
            return Result::OK;
        }

        return Result::E_SESSION_NONE;
    }

    void Store::setLimit( unsigned int limit ) {
        if( limit == 0 ) {
            _count = 0;
        }

        _limit = limit;
    }

    void Store::remove( const char *sessionId ) {
        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return;
        }

        _list[sessionId]->tsEnd = 0;
    }

    Store::Result Store::prolong( const char *sessionId, unsigned int lifetime ) {
        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

        if( lifetime == 0 ) {
            sess->tsEnd = 0;
        } else {
            sess->tsEnd = getTime() + lifetime;
        }

        return Result::OK;
    }

    Store::Result Store::addKey(
        const char *sessionId,
        const char *key,
        const char *value,
        unsigned int &counterKeys,
        unsigned int &counterRecord,
        unsigned int lifetime,
        unsigned short int limitWrite,
        unsigned short int limitRead
    ) {
        auto tsEndKey = getTime() + lifetime;

        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

        if( sess->tsEnd < tsEndKey ) {
            return Result::E_LIFETIME;
        }

        if( sess->values.find( key ) != sess->values.end() ) {
            return Result::E_DUPLICATE_KEY;
        }

        sess->counterKeys++;

        auto val = std::make_unique<Value>();
        val->value = value;
        if( lifetime != 0 ) {
            val->tsEnd = tsEndKey;
        }

        if( limitWrite != 0 ) {
            val->limiterWrite = std::make_unique<Limiter>();
            val->limiterWrite->limit = limitWrite;
        }

        if( limitRead != 0 ) {
            val->limiterRead = std::make_unique<Limiter>();
            val->limiterRead->limit = limitRead;
        }

        counterKeys = sess->counterKeys;
        counterRecord = 0;
        sess->values[key] = std::move( val );

        return Result::OK;
    }

    Store::Result Store::existKey( const char *sessionId, const char *key ) {
        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

        if( sess->values.find( key ) != sess->values.end() ) {
            return Result::E_KEY_NONE;
        }

        return Result::OK;
    }

    Store::Result Store::prolongKey( const char *sessionId, const char *key, unsigned int lifetime ) {
        auto tsEndKey = getTime() + lifetime;

        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

        if( sess->tsEnd < tsEndKey ) {
            return Result::E_LIFETIME;
        }

        if( sess->values.find( key ) == sess->values.end() ) {
            return Result::E_KEY_NONE;
        }

        auto val = sess->values[key].get();

        if( lifetime == 0 ) {
            val->tsEnd = 0;
        } else {
            val->tsEnd = tsEndKey;
        }

        return Result::OK;
    }


    Store::Result Store::setKey(
        const char *sessionId,
        const char *key,
        const char *value,
        unsigned int length,
        unsigned int counterKeys,
        unsigned int counterRecord
    ) {
        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

        if( sess->values.find( key ) == sess->values.end() ) {
            return Result::E_KEY_NONE;
        }

        auto val = sess->values[key].get();
        
        if( val->tsEnd != 0 && val->tsEnd < getTime() ) {
            return Result::E_KEY_NONE;
        }

        if( val->counterRecord != counterRecord || sess->counterKeys != counterKeys ) {
            return Result::E_RECORD_BEEN_CHANGED;
        }

        if( !incLimiter( val->limiterWrite.get() ) ) {
            return Result::E_LIMIT_PER_SEC;
        }

        val->value = std::string( value, length );
        val->counterRecord++;

        return Result::OK;
    }

    Store::Result Store::setForceKey(
        const char *sessionId,
        const char *key,
        const char *value,
        unsigned int length
    ) {
        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

        if( sess->values.find( key ) == sess->values.end() ) {
            return Result::E_KEY_NONE;
        }

        auto val = sess->values[key].get();

        if( val->tsEnd != 0 && val->tsEnd < getTime() ) {
            return Result::E_KEY_NONE;
        }

        if( !incLimiter( val->limiterWrite.get() ) ) {
            return Result::E_LIMIT_PER_SEC;
        }

        val->value = std::string( value, length );
        val->counterRecord++;

        return Result::OK;
    }

    Store::Result Store::getKey( const char *sessionId, const char *key, std::string &value, unsigned int &counterKeys, unsigned int &counterRecord ) {
        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

        if( sess->values.find( key ) == sess->values.end() ) {
            return Result::E_KEY_NONE;
        }

        auto val = sess->values[key].get();

        if( val->tsEnd != 0 && val->tsEnd < getTime() ) {
            return Result::E_KEY_NONE;
        }

        if( !incLimiter( val->limiterRead.get() ) ) {
            return Result::E_LIMIT_PER_SEC;
        }

        value = val->value;
        counterRecord = val->counterRecord;
        counterKeys = sess->counterKeys;

        return Result::OK;
    }

    Store::Result Store::removeKey( const char *sessionId, const char *key ) {
        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

        sess->values.erase( key );
        return Result::OK;
    }

    void Store::clearInactive() {
        auto tsCur = getTime();

        for( auto it = _list.begin(); it != _list.end(); ) {
            auto sess = _list[it->first].get();

            if( sess->tsEnd < tsCur && sess->tsEnd != 0 ) {
                it = _list.erase( it );
                _count--;
            } else {
                ++it;
                for( auto itV = sess->values.begin(); itV != sess->values.end(); ) {
                    auto val = sess->values[itV->first].get();
                    if( val->tsEnd != 0 && val->tsEnd < tsCur ) {
                        itV = sess->values.erase( itV );
                    } else {
                        ++itV;
                    }
                }
            }
        }
    }

    bool Store::incLimiter( Limiter *limiter ) {
        if( limiter == nullptr ) {
            return true;
        }

        if( limiter->limit == limiter->count && limiter->ts == getTime() ) {
            return false;
        } else if( limiter->ts != getTime() ) {
            limiter->count = 1;
        } else {
            limiter->count++;
        }

        return true;
    }

    Store::Result Store::setLimitToReadPerSec( const char *sessionId, const char *key, unsigned int limit ) {
        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

        if( sess->values.find( key ) == sess->values.end() ) {
            return Result::E_KEY_NONE;
        }

        auto val = sess->values[key].get();
        
        if( val->tsEnd != 0 && val->tsEnd < getTime() ) {
            return Result::E_KEY_NONE;
        }

        if( limit != 0 ) {
            val->limiterRead = std::make_unique<Limiter>();
            val->limiterRead->limit = limit;
        } else {
            val->limiterRead.reset();
        }

        return Result::OK;
    }

    Store::Result Store::setLimitToWritePerSec( const char *sessionId, const char *key, unsigned int limit ) {
        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

        if( sess->values.find( key ) == sess->values.end() ) {
            return Result::E_KEY_NONE;
        }

        auto val = sess->values[key].get();
        
        if( val->tsEnd != 0 && val->tsEnd < getTime() ) {
            return Result::E_KEY_NONE;
        }

        if( limit != 0 ) {
            val->limiterWrite = std::make_unique<Limiter>();
            val->limiterWrite->limit = limit;
        } else {
            val->limiterWrite.reset();
        }

        return Result::OK;
    }
}

#endif
