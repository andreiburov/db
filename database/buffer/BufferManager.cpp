#include <cassert>
#include <cstring>
#include "BufferManager.h"

BufferManager::BufferManager(uint64_t pages_in_ram) {
    registry_.reserve(pages_in_ram);
    cache_ = (page_t*)malloc(pages_in_ram*sizeof(page_t));
    memset(cache_, '\0', pages_in_ram*sizeof(page_t));
    for (int i = 0; i < pages_in_ram; ++i) {
        uint64_t page = (uint64_t)(i*sizeof(page_t));
        registry_[page] = new BufferFrame(true, false, i, this, page);
        fifo_.push_back(page);
        page_io_.readPage(page, registry_[page]->getData(), sizeof(page_t));
    }

#ifdef LOGGING
    file.open("mng.txt", std::ios::trunc);
#endif
}

BufferManager::~BufferManager() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto i = fifo_.cbegin(); i != fifo_.cend();) {
        if (registry_[*i]->dirty_) {
            page_io_.writePage(registry_[*i]->page_id_, registry_[*i]->getData(), sizeof(page_t));
        }
        i = fifo_.erase(i);
    }
    for (auto i = registry_.cbegin(); i != registry_.cend();) {
        i = registry_.erase(i);
    }
    free(cache_);
}

BufferFrame &BufferManager::fixPage(uint64_t page_id, bool exclusive) {
    uint64_t page_offset = GetPageOffset(page_id);
    uint64_t segment_id = GetSegment(page_id);

    std::unique_lock<std::mutex> lock(mutex_);
    BufferFrame* frame = registry_[page_id];

#ifdef LOGGING
    file << "page " << page_id << " fix\n";
#endif

    if (frame != nullptr) { // already created
        if (frame->loaded_) {
            assert(frame->i_ < fifo_.size());
            cond_.wait(lock, [&]{ return frame->tryLock(exclusive); });
#ifdef LOGGING
            file << "page " << page_id << " index " << frame->i_
                 << " cache " << ((unsigned*)&cache_[frame->i_])[0] << " in ram\n";
#endif
            lock.unlock();
        } else {
            putInCache(frame, lock, exclusive);
        }
    } else {
        frame = new BufferFrame(this, page_id);
        registry_[page_id] = frame;
#ifdef LOGGING
        file << "page " << page_id << " created " << frame->getData() << "\n";
#endif
        putInCache(frame, lock, exclusive);
    }

    return *frame;
}

bool BufferManager::findSlotInCache(BufferFrame* frame, BufferFrame* &slot) {
    if (!frame->tryLock(true)) {
        return false;
    }
    for (auto i = fifo_.cbegin(); i != fifo_.cend();) {
        if (!registry_[*i]->tryLock(true)) {
            ++i;
            continue;
        } else {
            slot = registry_[*i];
            //assert(slot->i_ < fifo_.size());
#ifdef LOGGING
            file << "page " << frame->page_id_ << " index " << frame->i_
                 << " slot " << slot->page_id_ << " index " << slot->i_ << "\n";
#endif
            i = fifo_.erase(i);
            fifo_.push_back(frame->page_id_);
            return true;
        }
    }
    return false;
}

void BufferManager::putInCache(BufferFrame *frame, std::unique_lock<std::mutex>& lock, bool exclusive) {
    BufferFrame* slot;
    cond_.wait(lock, [&]{ return findSlotInCache(frame, slot); });
    unsigned id = slot->i_;
    assert(id < fifo_.size());
#ifdef LOGGING
    file << "page " << slot->page_id_ << " index " << id
         << " to disk " << ((unsigned*)&cache_[id])[0] << "\n";
#endif
    slot->loaded_ = false;
    slot->i_ = -1;
    lock.unlock();
    if (slot->dirty_) {
        page_io_.writePage(slot->page_id_, &cache_[id], sizeof(page_t));
    }
    page_io_.readPage(frame->page_id_, &cache_[id], sizeof(page_t));
    lock.lock();
#ifdef LOGGING
    file << "page " << frame->page_id_ << " index " << id
         << " from disk " << ((unsigned*)&cache_[id])[0] << "\n";
#endif
    slot->dirty_ = false;
    //assert(frame->dirty_ == false);
    frame->i_ = id;
    frame->loaded_ = true;
    slot->unlock();
    frame->unlock();
    frame->blockingLock(exclusive);
#ifdef LOGGING
    file << "page " << frame->page_id_ << " index " << frame->i_
         << " cache " << ((unsigned*)&cache_[frame->i_])[0] << " in cache\n";
#endif
    lock.unlock();
    cond_.notify_one();
}

void BufferManager::unfixPage(BufferFrame &frame, bool is_dirty) {
    std::unique_lock<std::mutex> lock(mutex_);
    frame.dirty_ |= is_dirty;
    frame.unlock();
#ifdef LOGGING
    file << "page " << frame.page_id_ << " unfixed " << ((unsigned*)&cache_[frame.i_])[0] << "\n";
#endif
    lock.unlock();
    cond_.notify_one();
}


