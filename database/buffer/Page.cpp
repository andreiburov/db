#include <cstdint>
#include "Page.h"

uint64_t GetSegment(uint64_t page_id) {
    return page_id >> PAGE_MASK_IN_BITS;
}

uint64_t GetPageOffset(uint64_t page_id) {
    return page_id & PAGE_MASK;
}

uint64_t GetFileOffset(uint64_t page_offset) {
    return page_offset*PAGESIZE;
}

uint64_t GetFirstPage(uint64_t segment_id) {
    return segment_id << PAGE_MASK_IN_BITS;
}

uint64_t GetPageSize() {
    return PAGESIZE;
}
