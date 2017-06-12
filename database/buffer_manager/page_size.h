#pragma once

#include <stdint.h>
#include <pthread.h>

#define PAGESIZE 4

// 256 MB per file is 2^16 * 4KB
#define PAGE_MASK 0xffffUL
#define PAGE_SIZE_IN_BITS 16

uint64_t GetSegment(uint64_t page_id);
uint64_t GetOffset(uint64_t page_id);
uint64_t GetFirstPage(uint64_t segment_id);

typedef struct { char a[PAGESIZE]; } page_t;
