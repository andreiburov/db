#include "page_size.h"

uint64_t GetSegment(uint64_t page_id) { return page_id >> PAGE_SIZE_IN_BITS; }
uint64_t GetOffset(uint64_t page_id) { return page_id & PAGE_MASK; }
uint64_t GetFirstPage(uint64_t segment_id) { return segment_id << PAGE_SIZE_IN_BITS; }
