#ifndef MEMSESS_UTIL_LOCK_ATOMIC
#define MEMSESS_UTIL_LOCK_ATOMIC

#include <atomic>

namespace memsess::util {
    class LockAtomic {
        private:
        std::atomic_uint *atom;
     
        public:
        LockAtomic( std::atomic_uint &v ) {
            v++;
            atom = &v;
        }
     
        ~LockAtomic() {
            (*atom)--;
        }
    };
}


#endif
