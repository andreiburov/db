#ifndef DB_BUFFERFRAME_H
#define DB_BUFFERFRAME_H

#include <pthread.h>
#include <cstdint>

class BufferManager;

class BufferFrame {

    friend BufferManager;

    bool loaded_;
    bool dirty_;
    unsigned i_; // offset in cache
    BufferManager* buffer_manager_;
    uint64_t page_id_;
    pthread_rwlock_t rwlock_;

public:

    BufferFrame()
    {
        pthread_rwlock_init(&rwlock_, NULL);
    }

    BufferFrame(BufferManager* buffer_manager, uint64_t page_id)
            : loaded_(false), dirty_(false), i_(-1), buffer_manager_(buffer_manager), page_id_(page_id)
    {
        pthread_rwlock_init(&rwlock_, NULL);
    }

    BufferFrame(bool loaded, bool dirty, unsigned i, BufferManager* buffer_manager, uint64_t page_id)
            : loaded_(loaded), dirty_(dirty), i_(i), buffer_manager_(buffer_manager), page_id_(page_id)
    {
        pthread_rwlock_init(&rwlock_, NULL);
    }

    ~BufferFrame()
    {
        pthread_rwlock_destroy(&rwlock_);
    }

    void* getData();

    bool blockingLock(bool exclusive);

    bool tryLock(bool exclusive);

    void unlock();
};

#endif //DB_BUFFERFRAME_H
