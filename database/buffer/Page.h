#ifndef DB_PAGE_H
#define DB_PAGE_H

//size of segment 2^16 * PAGESIZE
const int PAGESIZE = 4096;
const int PAGE_MASK_IN_BITS = 16;
const int PAGE_MASK = ((1<<PAGE_MASK_IN_BITS)-1);

uint64_t GetSegment(uint64_t page_id);
uint64_t GetPageOffset(uint64_t page_id);
uint64_t GetFileOffset(uint64_t page_offset);
uint64_t GetFirstPage(uint64_t segment_id);
uint64_t GetPageSize();

typedef struct { char a[PAGESIZE]; } page_t;

#endif //DB_PAGE_H
