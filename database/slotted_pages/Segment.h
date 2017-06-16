#ifndef DB_SEGMENT_H
#define DB_SEGMENT_H

#include <atomic>
#include "../buffer/BufferManager.h"

class Segment {

protected:

    BufferManager& buffer_manager_;
    const uint64_t segment_id_;
    std::atomic<uint64_t> max_page_;
    std::mutex mutex_;

public:

    Segment(BufferManager& buffer_manager, uint64_t segment_id)
            : buffer_manager_(buffer_manager), segment_id_(segment_id), max_page_(0) {}
};

#endif //DB_SEGMENT_H