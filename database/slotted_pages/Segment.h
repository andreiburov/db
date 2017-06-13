#ifndef DB_SEGMENT_H
#define DB_SEGMENT_H

#include "../buffer/BufferManager.h"

class Segment {

protected:

    BufferManager& buffer_manager_;
    const uint64_t segment_id_;

public:

    Segment(BufferManager& buffer_manager, uint64_t segment_id)
            : buffer_manager_(buffer_manager), segment_id_(segment_id) {}
};

#endif //DB_SEGMENT_H