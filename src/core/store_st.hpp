#ifndef MEMSESS_CORE_STORE_ST
#define MEMSESS_CORE_STORE_ST

#include <memory>
#include <mutex>
#include <unordered_map>
#include <shared_mutex>
#include <atomic>
#include <string>
#include <string.h>
#include <time.h>
#include "../interfaces/store_interface.h"
#include "../util/lock_atomic.hpp"
#include "../util/uuid.hpp"

namespace memsess::core {
    class StoreST: public i::StoreInterface {

        public:
            struct Limiter {
                unsigned long int ts; 
                unsigned int limit;
                unsigned int count;
                std::mutex m;
            };
 
            struct Value {
                std::string value;
                std::atomic_uint writers;
                std::shared_timed_mutex m;
                unsigned long int tsEnd;
                unsigned int counterRecord;
                std::unique_ptr<Limiter> limiterRead;
                std::unique_ptr<Limiter> limiterWrite;
            };
 
            struct Item {
                std::unordered_map<std::string, std::unique_ptr<Value>> values;
                std::atomic_uint writers;
                std::shared_timed_mutex m;
                unsigned int counterKeys;
                unsigned long int tsEnd;
            };
 
        private:
            std::unordered_map<std::string, std::unique_ptr<Item>> _list;
            std::atomic_uint _writers{0};
            std::shared_timed_mutex _m;
            unsigned int _limit;
            unsigned int _count = 0;
            void _wait( std::atomic_uint &atom );
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
         
            void clearInactive();
            Result setLimitToReadPerSec( const char *sessionId, const char *key, unsigned int limit );
            Result setLimitToWritePerSec( const char *sessionId, const char *key, unsigned int limit );
    };

    unsigned long int StoreST::getTime() {
        return time( NULL );
    }

    void StoreST::_wait( std::atomic_uint &atom ) {
        while( atom );
    }

    StoreST::Result StoreST::generate( unsigned int lifetime, char *sessionId ) {
        util::LockAtomic lock( _writers );
        std::lock_guard<std::shared_timed_mutex> lockList( _m );
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

    StoreST::Result StoreST::exist( const char *sessionId ) {
        _wait( _writers );
        std::shared_lock<std::shared_timed_mutex> lockList( _m );

        if( _list.find( sessionId ) != _list.end() ) {
            return Result::OK;
        }

        return Result::E_SESSION_NONE;
    }

    void StoreST::setLimit( unsigned int limit ) {
        util::LockAtomic lock( _writers );
        std::lock_guard<std::shared_timed_mutex> lockList( _m );

        if( limit == 0 ) {
            _count = 0;
        }

        _limit = limit;
    }

    void StoreST::remove( const char *sessionId ) {
        _wait( _writers );
        std::shared_lock<std::shared_timed_mutex> lockList( _m );

        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return;
        }

        _list[sessionId]->tsEnd = 0;
    }

    StoreST::Result StoreST::prolong( const char *sessionId, unsigned int lifetime ) {

        _wait( _writers );
        std::shared_lock<std::shared_timed_mutex> lockList( _m );

        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

        util::LockAtomic writersValues( sess->writers );
        std::lock_guard<std::shared_timed_mutex> lockValues( sess->m );

        if( lifetime != 0 ) {
            sess->tsEnd = getTime() + lifetime;
        } else {
            sess->tsEnd = 0;
        }

        return Result::OK;
    }

    StoreST::Result StoreST::addKey(
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

        _wait( _writers );
        std::shared_lock<std::shared_timed_mutex> lockList( _m );

        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

        if( sess->tsEnd < tsEndKey ) {
            return Result::E_LIFETIME;
        }

        util::LockAtomic writersValues( sess->writers );
        std::lock_guard<std::shared_timed_mutex> lockValues( sess->m );

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

    StoreST::Result StoreST::existKey( const char *sessionId, const char *key ) {
        _wait( _writers );
        std::shared_lock<std::shared_timed_mutex> lockList( _m );

        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

        _wait( sess->writers );
        std::shared_lock<std::shared_timed_mutex> lockValues( sess->m );

        if( sess->values.find( key ) != sess->values.end() ) {
            return Result::E_KEY_NONE;
        }

        return Result::OK;
    }

    StoreST::Result StoreST::prolongKey( const char *sessionId, const char *key, unsigned int lifetime ) {
        auto tsEndKey = getTime() + lifetime;

        _wait( _writers );
        std::shared_lock<std::shared_timed_mutex> lockList( _m );

        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

        if( sess->tsEnd < tsEndKey ) {
            return Result::E_LIFETIME;
        }

        _wait( sess->writers );
        std::shared_lock<std::shared_timed_mutex> lockValues( sess->m );

        if( sess->values.find( key ) == sess->values.end() ) {
            return Result::E_KEY_NONE;
        }

        auto val = sess->values[key].get();

        util::LockAtomic writersValue( val->writers );
        std::lock_guard<std::shared_timed_mutex> lockValue( val->m );

        if ( lifetime != 0 ) {
            val->tsEnd = tsEndKey;
        } else {
            val->tsEnd = 0;
        }

        return Result::OK;
    }


    StoreST::Result StoreST::setKey(
        const char *sessionId,
        const char *key,
        const char *value,
        unsigned int length,
        unsigned int counterKeys,
        unsigned int counterRecord
    ) {
        _wait( _writers );
        std::shared_lock<std::shared_timed_mutex> lockList( _m );

        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

        _wait( sess->writers );
        std::shared_lock<std::shared_timed_mutex> lockValues( sess->m );

        if( sess->values.find( key ) == sess->values.end() ) {
            return Result::E_KEY_NONE;
        }

        auto val = sess->values[key].get();
        
        if( val->tsEnd != 0 && val->tsEnd < getTime() ) {
            return Result::E_KEY_NONE;
        }

        util::LockAtomic writersValue( val->writers );
        std::lock_guard<std::shared_timed_mutex> lockValue( val->m );

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

    StoreST::Result StoreST::setForceKey(
        const char *sessionId,
        const char *key,
        const char *value,
        unsigned int length
    ) {
        _wait( _writers );
        std::shared_lock<std::shared_timed_mutex> lockList( _m );

        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

        _wait( sess->writers );
        std::shared_lock<std::shared_timed_mutex> lockValues( sess->m );

        if( sess->values.find( key ) == sess->values.end() ) {
            return Result::E_KEY_NONE;
        }

        auto val = sess->values[key].get();

        if( val->tsEnd != 0 && val->tsEnd < getTime() ) {
            return Result::E_KEY_NONE;
        }

        util::LockAtomic writersValue( val->writers );
        std::lock_guard<std::shared_timed_mutex> lockValue( val->m );

        if( !incLimiter( val->limiterWrite.get() ) ) {
            return Result::E_LIMIT_PER_SEC;
        }

        val->value = std::string( value, length );
        val->counterRecord++;

        return Result::OK;
    }

    StoreST::Result StoreST::getKey( const char *sessionId, const char *key, std::string &value, unsigned int &counterKeys, unsigned int &counterRecord ) {
        _wait( _writers );
        std::shared_lock<std::shared_timed_mutex> lockList( _m );

        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

        _wait( sess->writers );
        std::shared_lock<std::shared_timed_mutex> lockValues( sess->m );

        if( sess->values.find( key ) == sess->values.end() ) {
            return Result::E_KEY_NONE;
        }

        auto val = sess->values[key].get();

        if( val->tsEnd != 0 && val->tsEnd < getTime() ) {
            return Result::E_KEY_NONE;
        }

        _wait( val->writers );
        std::shared_lock<std::shared_timed_mutex> lockValue( val->m );

        if( !incLimiter( val->limiterRead.get() ) ) {
            return Result::E_LIMIT_PER_SEC;
        }

        value = val->value;
        counterRecord = val->counterRecord;
        counterKeys = sess->counterKeys;

        return Result::OK;
    }

    StoreST::Result StoreST::removeKey( const char *sessionId, const char *key ) {
        _wait( _writers );
        std::shared_lock<std::shared_timed_mutex> lockList( _m );

        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

        util::LockAtomic writersValues( sess->writers );
        std::lock_guard<std::shared_timed_mutex> lockValues( sess->m );

        sess->values.erase( key );

        return Result::OK;
    }

    void StoreST::clearInactive() {
        util::LockAtomic lock( _writers );
        std::lock_guard<std::shared_timed_mutex> lockList( _m );

        auto tsCur = getTime();

        for( auto it = _list.begin(); it != _list.end(); ++it ) {
            auto sess = _list[it->first].get();

            if( sess->tsEnd < tsCur && sess->tsEnd != 0 ) {
                _list.erase( it++ );
                _count--;
            } else {
                for( auto itV = sess->values.begin(); itV != sess->values.end(); ++itV ) {
                    auto val = sess->values[itV->first].get();
                    if( val->tsEnd != 0 && val->tsEnd < tsCur ) {
                        sess->values.erase( itV++ );
                    }
                }
            }
        }
    }

    bool StoreST::incLimiter( Limiter *limiter ) {
        if( limiter == nullptr ) {
            return true;
        }

        std::lock_guard<std::mutex> lock( limiter->m );

        if( limiter->limit == limiter->count && limiter->ts == getTime() ) {
            return false;
        } else if( limiter->ts != getTime() ) {
            limiter->count = 1;
        } else {
            limiter->count++;
        }

        return true;
    }

    StoreST::Result StoreST::setLimitToReadPerSec( const char *sessionId, const char *key, unsigned int limit ) {
        _wait( _writers );
        std::shared_lock<std::shared_timed_mutex> lockList( _m );

        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

        _wait( sess->writers );
        std::shared_lock<std::shared_timed_mutex> lockValues( sess->m );

        if( sess->values.find( key ) == sess->values.end() ) {
            return Result::E_KEY_NONE;
        }

        auto val = sess->values[key].get();
        
        if( val->tsEnd != 0 && val->tsEnd < getTime() ) {
            return Result::E_KEY_NONE;
        }

        util::LockAtomic writersValue( val->writers );
        std::lock_guard<std::shared_timed_mutex> lockValue( val->m );

        val->limiterRead = std::make_unique<Limiter>();
        val->limiterRead->limit = limit;

        return Result::OK;
    }

    StoreST::Result StoreST::setLimitToWritePerSec( const char *sessionId, const char *key, unsigned int limit ) {
        _wait( _writers );
        std::shared_lock<std::shared_timed_mutex> lockList( _m );

        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

        _wait( sess->writers );
        std::shared_lock<std::shared_timed_mutex> lockValues( sess->m );

        if( sess->values.find( key ) == sess->values.end() ) {
            return Result::E_KEY_NONE;
        }

        auto val = sess->values[key].get();
        
        if( val->tsEnd != 0 && val->tsEnd < getTime() ) {
            return Result::E_KEY_NONE;
        }

        util::LockAtomic writersValue( val->writers );
        std::lock_guard<std::shared_timed_mutex> lockValue( val->m );

        val->limiterWrite = std::make_unique<Limiter>();
        val->limiterWrite->limit = limit;

        return Result::OK;
    }
}

#endif
