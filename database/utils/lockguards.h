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
    BufferFrame& frame;

    FrameGuard(BufferManager bm, uint64_t page_id, bool exclusive)
            : bm(bm), page_id(page_id), exclusive(exclusive), frame(bm.fixPage(page_id, exclusive))
    {}

    ~FrameGuard() {
        bm.unfixPage(frame, exclusive);
    }
};

#endif //DB_LOCKGUARDS_H