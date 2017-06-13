#include "gtest/gtest.h"
#include "../../database/dbms.h"

const char* str1 = "Hello World!";
const char* str2 = "X";
const char* str3 = "Why are you looking at me? Go away!";
const char* str4 = "Property is mince?";

const int record_count = 4;

Record r[record_count] = {Record(strlen(str1), str1), Record(strlen(str2), str2),
            Record(strlen(str3), str3), Record(strlen(str4), str4)};

TEST(SPSegment, InsertLookup) {
    BufferManager bm(2);
    SPSegment segment(bm, 1);
    TID tid1 = segment.insert(r[0]);
    TID tid2 = segment.insert(r[1]);
    
    EXPECT_EQ(memcmp(r[0].getData(), segment.lookup(tid1).getData(), r[0].getLen()),0);
    EXPECT_EQ(memcmp(r[0].getData(), segment.lookup(tid2).getData(), r[0].getLen()),0);
}

TEST(SPSegment, Compactify) {
    unsigned segment_id = 1;
    BufferManager bm(30);
    SPSegment segment(bm, segment_id);

    uint64_t offset = PAGESIZE;
    uint64_t expected = PAGESIZE;
    unsigned max_pad = 100;
    FrameGuard guard(bm, GetFirstPage(segment_id), true);
    char* data = reinterpret_cast<char*>(guard.frame.getData());
    SPSegment::Header* header = new (data) SPSegment::Header();
    header->slot_count = record_count;

    for (int i = 0; i < record_count; ++i) {
        header->data_start -= rand()%max_pad;
        SPSegment::Slot* slot = new(segment.getFirstSlot(header)+ i) SPSegment::Slot();
        segment.insert(r[i], header, slot);
        expected -= r[i].getLen();
    }

    segment.compactify(header);
    ASSERT_EQ(expected, header->data_start);

    for (int i = 0; i < record_count; ++i) {
        //EXPECT_STREQ(r[i].getData(),
        //             reinterpret_cast<char*>(data) + segment.getFirstSlot(header)[i].offset);
        EXPECT_EQ(memcmp(r[i].getData(),
                         reinterpret_cast<char*>(data) + segment.getFirstSlot(header)[i].offset,
                         r[i].getLen()), 0);
    }
}
