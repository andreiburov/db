#ifndef DB_SEGMENT_H
#define DB_SEGMENT_H

#include <atomic>
#include "../buffer/BufferManager.h"

class Segment {

protected:

    BufferManager& buffer_manager_;
    const uint64_t segment_id_;
    std::atomic<uint64_t> max_page_;
    std::atomic<uint64_t> size_;
    std::mutex mutex_;

public:

    Segment(BufferManager& buffer_manager, uint64_t segment_id)
            : buffer_manager_(buffer_manager), segment_id_(segment_id), max_page_(0), size_(0) {}

    inline BufferManager& getBufferManager() { return buffer_manager_; }

    inline uint64_t getSegmentId() const { return segment_id_; }

    inline uint64_t getMaxPageId() const { return max_page_; }

    inline uint64_t size() const { return size_; }
};

#endif //DB_SEGMENT_H