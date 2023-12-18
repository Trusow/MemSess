#ifndef MEMSESS_CORE_STORE_ST
#define MEMSESS_CORE_STORE_ST

#include <memory>
#include <unordered_map>

#if MEMSESS_MULTI
#include <mutex>
#include <shared_mutex>
#include <atomic>
#endif

#include <string>
#include <string.h>
#include <time.h>
#include "../interfaces/store_interface.h"
#include "../util/uuid.hpp"

#if MEMSESS_MULTI
#include "../util/lock_atomic.hpp"
#endif


namespace memsess::core {
    class Store: public i::StoreInterface {

        public:
            struct Limiter {
                unsigned long int ts; 
                unsigned int count;
#if MEMSESS_MULTI
                std::mutex m;
#endif
            };
 
            struct Value {
                std::string value;
#if MEMSESS_MULTI
                std::atomic_uint writers;
                std::shared_timed_mutex m;
#endif
                unsigned long int tsEnd;
                unsigned int counterRecord;
                std::unique_ptr<Limiter> limiterRead;
                std::unique_ptr<Limiter> limiterWrite;
            };
 
            struct Item {
                std::unordered_map<std::string, std::unique_ptr<Value>> values;
#if MEMSESS_MULTI
                std::atomic_uint writers;
                std::shared_timed_mutex m;
#endif
                unsigned int counterKeys;
                unsigned long int tsEnd;
            };
 
        private:
            std::unordered_map<std::string, std::unique_ptr<Item>> _list;
#if MEMSESS_MULTI
            std::atomic_uint _writers{0};
            std::shared_timed_mutex _m;
#endif
            unsigned int _limit;
            unsigned int _count = 0;
#if MEMSESS_MULTI
            void _wait( std::atomic_uint &atom );
#endif
            unsigned long int getTime();
            bool incLimiter( Limiter *limiter, unsigned short int limit );
 
        public:
            Result add( const char *sessionId );
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
                unsigned int lifetime = 0
            );
            Result existKey( const char *sessionId, const char *key );
            Result prolongKey( const char *sessionId, const char *key, unsigned int lifetime );
            Result setKey(
                const char *sessionId,
                const char *key,
                const char *value,
                unsigned int length,
                unsigned int counterKeys,
                unsigned int counterRecord,
                unsigned short int limit = 0
            );
            Result setForceKey(
                const char *sessionId,
                const char *key,
                const char *value,
                unsigned int length,
                unsigned short int limit = 0
            );
            Result getKey(
                const char *sessionId,
                const char *key,
                std::string &value,
                unsigned int &counterKeys,
                unsigned int &counterRecord,
                unsigned short int limit = 0
            );
            Result removeKey( const char *sessionId, const char *key );
         
            void clearInactive();
            Result addAllKey(
                const char *key,
                const char *value
            );
            Result removeAllKey( const char *key );
    };

    unsigned long int Store::getTime() {
        return time( NULL );
    }

#if MEMSESS_MULTI
    void Store::_wait( std::atomic_uint &atom ) {
        while( atom );
    }
#endif

    Store::Result Store::add( const char *sessionId ) {
#if MEMSESS_MULTI
        util::LockAtomic lock( _writers );
        std::lock_guard<std::shared_timed_mutex> lockList( _m );
#endif
        if( ( _count == _limit && _limit != 0 ) || _count == 0xFF'FF'FF'FF ) {
            return Result::E_LIMIT;
        }

        if( _list.find( sessionId ) != _list.end() ) {
            return Result::E_DUPLICATE_SESSION;
        }

        auto item = std::make_unique<Item>();

        item->tsEnd = 0xFFFFFFFF;

        _list[sessionId] = std::move( item );
        _count++;

        return Result::OK;
    }

    Store::Result Store::generate( unsigned int lifetime, char *sessionId ) {
#if MEMSESS_MULTI
        util::LockAtomic lock( _writers );
        std::lock_guard<std::shared_timed_mutex> lockList( _m );
#endif
        if( ( _count == _limit && _limit != 0 ) || _count == 0xFF'FF'FF'FF ) {
            return Result::E_LIMIT;
        }

        while( true ) {
            util::UUID::generate( sessionId );

            if( _list.find( sessionId ) == _list.end() ) {
                break;
            }
        }

        auto item = std::make_unique<Item>();

        if( lifetime != 0 ) {
            item->tsEnd = getTime() + lifetime;
        } else {
            item->tsEnd = 0xFFFFFFFF;
        }

        _list[sessionId] = std::move( item );
        _count++;

        return Result::OK;
    }

    Store::Result Store::exist( const char *sessionId ) {
#if MEMSESS_MULTI
        _wait( _writers );
        std::shared_lock<std::shared_timed_mutex> lockList( _m );
#endif

        if( _list.find( sessionId ) != _list.end() ) {
            return Result::OK;
        }

        return Result::E_SESSION_NONE;
    }

    void Store::setLimit( unsigned int limit ) {
#if MEMSESS_MULTI
        util::LockAtomic lock( _writers );
        std::lock_guard<std::shared_timed_mutex> lockList( _m );
#endif

        if( limit == 0 ) {
            _count = 0;
        }

        _limit = limit;
    }

    void Store::remove( const char *sessionId ) {
#if MEMSESS_MULTI
        _wait( _writers );
        std::shared_lock<std::shared_timed_mutex> lockList( _m );
#endif

        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return;
        }

        _list[sessionId]->tsEnd = 0;
    }

    Store::Result Store::prolong( const char *sessionId, unsigned int lifetime ) {
#if MEMSESS_MULTI
        _wait( _writers );
        std::shared_lock<std::shared_timed_mutex> lockList( _m );
#endif

        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

#if MEMSESS_MULTI
        util::LockAtomic writersValues( sess->writers );
        std::lock_guard<std::shared_timed_mutex> lockValues( sess->m );
#endif

        if( lifetime != 0 ) {
            sess->tsEnd = getTime() + lifetime;
        } else {
            sess->tsEnd = 0xFFFFFFFF;
        }

        return Result::OK;
    }

    Store::Result Store::addKey(
        const char *sessionId,
        const char *key,
        const char *value,
        unsigned int &counterKeys,
        unsigned int &counterRecord,
        unsigned int lifetime
    ) {
        auto tsEndKey = getTime() + lifetime;

#if MEMSESS_MULTI
        _wait( _writers );
        std::shared_lock<std::shared_timed_mutex> lockList( _m );
#endif

        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

        if( sess->tsEnd < tsEndKey ) {
            return Result::E_LIFETIME;
        }

#if MEMSESS_MULTI
        util::LockAtomic writersValues( sess->writers );
        std::lock_guard<std::shared_timed_mutex> lockValues( sess->m );
#endif

        if( sess->values.find( key ) != sess->values.end() ) {
            return Result::E_DUPLICATE_KEY;
        }

        sess->counterKeys++;

        auto val = std::make_unique<Value>();
        val->value = value;

        if( lifetime != 0 ) {
            val->tsEnd = tsEndKey;
        }

        val->limiterWrite = std::make_unique<Limiter>();
        val->limiterRead = std::make_unique<Limiter>();

        counterKeys = sess->counterKeys;
        counterRecord = 0;
        sess->values[key] = std::move( val );

        return Result::OK;
    }

    Store::Result Store::existKey( const char *sessionId, const char *key ) {
#if MEMSESS_MULTI
        _wait( _writers );
        std::shared_lock<std::shared_timed_mutex> lockList( _m );
#endif

        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

#if MEMSESS_MULTI
        _wait( sess->writers );
        std::shared_lock<std::shared_timed_mutex> lockValues( sess->m );
#endif

        if( sess->values.find( key ) == sess->values.end() ) {
            return Result::E_KEY_NONE;
        }

        auto val = sess->values[key].get();
        
        if( val->tsEnd != 0 && val->tsEnd < getTime() ) {
            return Result::E_KEY_NONE;
        }

        return Result::OK;
    }

    Store::Result Store::prolongKey( const char *sessionId, const char *key, unsigned int lifetime ) {
        auto tsEndKey = getTime() + lifetime;

#if MEMSESS_MULTI
        _wait( _writers );
        std::shared_lock<std::shared_timed_mutex> lockList( _m );
#endif

        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

        if( sess->tsEnd < tsEndKey ) {
            return Result::E_LIFETIME;
        }

#if MEMSESS_MULTI
        _wait( sess->writers );
        std::shared_lock<std::shared_timed_mutex> lockValues( sess->m );
#endif

        if( sess->values.find( key ) == sess->values.end() ) {
            return Result::E_KEY_NONE;
        }

        auto val = sess->values[key].get();

        if( val->tsEnd != 0 && val->tsEnd < getTime() ) {
            return Result::E_KEY_NONE;
        }

#if MEMSESS_MULTI
        util::LockAtomic writersValue( val->writers );
        std::lock_guard<std::shared_timed_mutex> lockValue( val->m );
#endif

        if ( lifetime != 0 ) {
            val->tsEnd = tsEndKey;
        } else {
            val->tsEnd = 0;
        }

        return Result::OK;
    }


    Store::Result Store::setKey(
        const char *sessionId,
        const char *key,
        const char *value,
        unsigned int length,
        unsigned int counterKeys,
        unsigned int counterRecord,
        unsigned short int limit
    ) {
#if MEMSESS_MULTI
        _wait( _writers );
        std::shared_lock<std::shared_timed_mutex> lockList( _m );
#endif

        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

#if MEMSESS_MULTI
        _wait( sess->writers );
        std::shared_lock<std::shared_timed_mutex> lockValues( sess->m );
#endif

        if( sess->values.find( key ) == sess->values.end() ) {
            return Result::E_KEY_NONE;
        }

        auto val = sess->values[key].get();
        
        if( val->tsEnd != 0 && val->tsEnd < getTime() ) {
            return Result::E_KEY_NONE;
        }

#if MEMSESS_MULTI
        util::LockAtomic writersValue( val->writers );
        std::lock_guard<std::shared_timed_mutex> lockValue( val->m );
#endif

        if( val->counterRecord != counterRecord || sess->counterKeys != counterKeys ) {
            return Result::E_RECORD_BEEN_CHANGED;
        }

        if( !incLimiter( val->limiterWrite.get(), limit ) ) {
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
        unsigned int length,
        unsigned short int limit
    ) {
#if MEMSESS_MULTI
        _wait( _writers );
        std::shared_lock<std::shared_timed_mutex> lockList( _m );
#endif

        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

#if MEMSESS_MULTI
        _wait( sess->writers );
        std::shared_lock<std::shared_timed_mutex> lockValues( sess->m );
#endif

        if( sess->values.find( key ) == sess->values.end() ) {
            return Result::E_KEY_NONE;
        }

        auto val = sess->values[key].get();

        if( val->tsEnd != 0 && val->tsEnd < getTime() ) {
            return Result::E_KEY_NONE;
        }

#if MEMSESS_MULTI
        util::LockAtomic writersValue( val->writers );
        std::lock_guard<std::shared_timed_mutex> lockValue( val->m );
#endif

        if( !incLimiter( val->limiterWrite.get(), limit ) ) {
            return Result::E_LIMIT_PER_SEC;
        }

        val->value = std::string( value, length );
        val->counterRecord++;

        return Result::OK;
    }

    Store::Result Store::getKey(
        const char *sessionId,
        const char *key,
        std::string &value,
        unsigned int &counterKeys,
        unsigned int &counterRecord,
        unsigned short int limit
    ) {
#if MEMSESS_MULTI
        _wait( _writers );
        std::shared_lock<std::shared_timed_mutex> lockList( _m );
#endif

        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

#if MEMSESS_MULTI
        _wait( sess->writers );
        std::shared_lock<std::shared_timed_mutex> lockValues( sess->m );
#endif

        if( sess->values.find( key ) == sess->values.end() ) {
            return Result::E_KEY_NONE;
        }

        auto val = sess->values[key].get();

        if( val->tsEnd != 0 && val->tsEnd < getTime() ) {
            return Result::E_KEY_NONE;
        }

#if MEMSESS_MULTI
        _wait( val->writers );
        std::shared_lock<std::shared_timed_mutex> lockValue( val->m );
#endif

        if( !incLimiter( val->limiterRead.get(), limit ) ) {
            return Result::E_LIMIT_PER_SEC;
        }

        value = val->value;
        counterRecord = val->counterRecord;
        counterKeys = sess->counterKeys;

        return Result::OK;
    }

    Store::Result Store::removeKey( const char *sessionId, const char *key ) {
#if MEMSESS_MULTI
        _wait( _writers );
        std::shared_lock<std::shared_timed_mutex> lockList( _m );
#endif

        if( _list.find( sessionId ) == _list.end() || _list[sessionId]->tsEnd < getTime() ) {
            return Result::E_SESSION_NONE;
        }

        auto sess = _list[sessionId].get();

#if MEMSESS_MULTI
        util::LockAtomic writersValues( sess->writers );
        std::lock_guard<std::shared_timed_mutex> lockValues( sess->m );
#endif

        sess->values.erase( key );

        return Result::OK;
    }

    void Store::clearInactive() {
#if MEMSESS_MULTI
        util::LockAtomic lock( _writers );
        std::lock_guard<std::shared_timed_mutex> lockList( _m );
#endif

        auto tsCur = getTime();

        for( auto it = _list.begin(); it != _list.end(); ) {
            auto sess = _list[it->first].get();

            if( sess->tsEnd < tsCur ) {
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

    bool Store::incLimiter( Limiter *limiter, unsigned short int limit ) {
        if( limit == 0 ) {
            return true;
        }

#if MEMSESS_MULTI
        std::lock_guard<std::mutex> lock( limiter->m );
#endif

        if( limit == limiter->count && limiter->ts == getTime() ) {
            return false;
        } else if( limiter->ts != getTime() ) {
            limiter->ts = getTime();
            limiter->count = 1;
        } else {
            limiter->count++;
        }

        return true;
    }

    Store::Result Store::addAllKey(
        const char *key,
        const char *value
    ) {
#if MEMSESS_MULTI
        util::LockAtomic lock( _writers );
        std::lock_guard<std::shared_timed_mutex> lockList( _m );
#endif
        auto tsCur = getTime();

        for( auto it = _list.begin(); it != _list.end(); ++it ) {
            auto sess = _list[it->first].get();

            if( sess->tsEnd < tsCur || sess->values.find( key ) != sess->values.end() ) {
                continue;
            }

            auto val = std::make_unique<Value>();
            val->value = value;

            val->limiterWrite = std::make_unique<Limiter>();
            val->limiterRead = std::make_unique<Limiter>();

            sess->values[key] = std::move( val );
        }

        return Result::OK;
    }

    Store::Result Store::removeAllKey( const char *key ) {
#if MEMSESS_MULTI
        util::LockAtomic lock( _writers );
        std::lock_guard<std::shared_timed_mutex> lockList( _m );
#endif
        auto tsCur = getTime();

        for( auto it = _list.begin(); it != _list.end(); ++it ) {
            auto sess = _list[it->first].get();

            if( sess->tsEnd < tsCur ) {
                continue;
            }

            sess->values.erase( key );
        }

        return Result::OK;
    }
}

#endif
