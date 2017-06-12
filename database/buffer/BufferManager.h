#ifndef DB_BUFFERMANAGER_H
#define DB_BUFFERMANAGER_H

#include <mutex>
#include <unordered_map>
#include <list>
#include <condition_variable>
#include "BufferFrame.h"
#include "Page.h"
#include "PageIO.h"

class BufferManager {
    std::mutex mutex_;
    std::condition_variable cond_;
    page_t* cache_;
    PageIO page_io_;
    uint64_t cache_size_;

    // to synchronize
    std::unordered_map<uint64_t, BufferFrame*> registry_;
    std::list<uint64_t> fifo_;

public:
    BufferManager(uint64_t pages_in_ram);

    ~BufferManager();

    BufferFrame& fixPage(uint64_t page_id, bool exclusive);

    void unfixPage(BufferFrame& frame, bool is_dirty);

    inline page_t* getPage(unsigned i) const {
        return &cache_[i];
    }

private:

    bool findSlotInCache(BufferFrame* frame, BufferFrame* &slot);

    bool putInCache(BufferFrame* frame, std::unique_lock<std::mutex>& lock, bool exclusive);

};

#endif //DB_BUFFERMANAGER_H
