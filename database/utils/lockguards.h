#ifndef DB_LOCKGUARDS_H
#define DB_LOCKGUARDS_H

#include <pthread.h>

struct RWLockGuard {

    pthread_rwlock_t& rwlock_;
    bool locked;

    RWLockGuard(pthread_rwlock_t& rwlock) : rwlock_(rwlock), locked(false) {}

    ~RWLockGuard() {
        if (locked) {
            pthread_rwlock_unlock(&rwlock_);
        }
    }

    inline int trywrlock() {
        int ret = pthread_rwlock_trywrlock(&rwlock_);
        if (ret == 0) {
            locked = true;
        }
        return ret;
    }
};

#endif //DB_LOCKGUARDS_H