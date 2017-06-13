#ifndef DB_LOCKGUARDS_H
#define DB_LOCKGUARDS_H

#include <pthread.h>
#include "../buffer/BufferManager.h"

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

struct FrameGuard {

    BufferManager& bm;
    uint64_t page_id;
    bool exclusive;
    bool locked;
    BufferFrame& frame;

    FrameGuard(BufferManager& bm, uint64_t page_id, bool exclusive)
            : bm(bm), page_id(page_id), exclusive(exclusive), locked(true),
              frame(bm.fixPage(page_id, exclusive))
    {}

    bool lock(bool excl) {
        if (!locked) {
            bm.fixPage(page_id, excl);
            locked = true;
            exclusive = excl;
            return true;
        }
        return false;
    }

    inline bool lock() {
        lock(exclusive);
    }

    inline bool lock(const FrameGuard& guard) {
        if (page_id != guard.page_id) {
            lock();
        }
    }

    bool unlock(bool dirty) {
        if (locked) {
            bm.unfixPage(frame, dirty);
            locked = false;
        }
    }

    inline bool unlock() {
        unlock(exclusive);
    }

    ~FrameGuard() {
        if (locked) {
            bm.unfixPage(frame, exclusive);
        }
    }
};

#endif //DB_LOCKGUARDS_H