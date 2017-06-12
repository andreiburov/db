#include <cassert>
#include <cstring>
#include "BufferManager.h"

BufferManager::BufferManager(uint64_t pages_in_ram) : cache_size_(pages_in_ram)
{
    registry_.reserve(pages_in_ram);
    cache_ = (page_t*)malloc(pages_in_ram*sizeof(page_t));
    for (int i = 0; i < pages_in_ram; ++i) {
        uint64_t page = (uint64_t)(i*sizeof(page_t));
        registry_[page] = new BufferFrame(true, false, i, this, page);
        fifo_.push_back(page);
        page_io_.readPage(page, registry_[page]->getData(), sizeof(page_t));
    }
}

BufferManager::~BufferManager() {
    std::lock_guard<std::mutex> lock(mutex_);
    unsigned size = fifo_.size();
    for (auto i = fifo_.begin(); i != fifo_.end();) {
        if (registry_[*i]->dirty_) {
            page_io_.writePage(registry_[*i]->page_id_, registry_[*i]->getData(), sizeof(page_t));
        }
        i = fifo_.erase(i);
    }
    for (auto i = registry_.begin(); i != registry_.end();) {
        i = registry_.erase(i);
    }
    free(cache_);
}

BufferFrame &BufferManager::fixPage(uint64_t page_id, bool exclusive)
{
    uint64_t page_offset = GetPageOffset(page_id);
    uint64_t segment_id = GetSegment(page_id);
    BufferFrame* frame;

    do {
        std::unique_lock<std::mutex> lock(mutex_);
        frame = registry_[page_id];
        if (frame != nullptr) { // already created
            if (frame->loaded_) {
                assert(frame->i_ < cache_size_);
                cond_.wait(lock, [&]{ return frame->tryLock(exclusive); });

                if (!frame->loaded_) // someone unloaded the frame while we were waiting
                {
                    frame->unlock();
                    lock.unlock();
                    cond_.notify_one();
                }
                else { // frame still loaded
                    lock.unlock();
                    break; // loaded frame fixed
                }
            } else {
                if (putInCache(frame, lock, exclusive)) {
                    break; // cached frame fixed;
                }
            }
        } else {
            frame = new BufferFrame(this, page_id);
            registry_[page_id] = frame;

            if (putInCache(frame, lock, exclusive)) {
                break; // cached frame fixed
            }
        }
    } while (true);

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
            assert(slot->i_ < cache_size_);
            i = fifo_.erase(i);
            return true;
        }
    }
    frame->unlock();
    return false;
}

bool BufferManager::putInCache(BufferFrame *frame, std::unique_lock<std::mutex>& lock, bool exclusive)
{
    BufferFrame* slot;
    cond_.wait(lock, [&]{ return findSlotInCache(frame, slot); });

    if (frame->loaded_) {
        fifo_.push_front(slot->page_id_);
        frame->unlock();
        slot->unlock();
        lock.unlock();
        cond_.notify_one();
        return false;
    } else {
        unsigned id = slot->i_;
        assert(id < cache_size_);
        slot->loaded_ = false;
        slot->i_ = -1;
        lock.unlock();
        if (slot->dirty_) {
            page_io_.writePage(slot->page_id_, &cache_[id], sizeof(page_t));
        }
        page_io_.readPage(frame->page_id_, &cache_[id], sizeof(page_t));
        lock.lock();
        slot->dirty_ = false;
        assert(frame->dirty_ == false);
        frame->i_ = id;
        frame->loaded_ = true;
        fifo_.push_back(frame->page_id_);
        slot->unlock();
        frame->unlock();
        frame->blockingLock(exclusive);
        lock.unlock();
        cond_.notify_one();
        return true;
    }
}

void BufferManager::unfixPage(BufferFrame &frame, bool is_dirty)
{
    std::unique_lock<std::mutex> lock(mutex_);
    frame.dirty_ |= is_dirty;
    frame.unlock();
    lock.unlock();
    cond_.notify_one();
}


