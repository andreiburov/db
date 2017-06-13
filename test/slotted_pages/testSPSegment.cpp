#include "gtest/gtest.h"
#include "../../database/dbms.h"

TEST(SPSegment, InsertLookup) {
    const char* str1 = "Hello World!";
    const char* str2 = "X";
    Record r1(strlen(str1), str1);
    Record r2(strlen(str2), str2);

    BufferManager bm(2);
    SPSegment segment(bm, 1);
    TID tid1 = segment.insert(r1);
    TID tid2 = segment.insert(r2);
    
    EXPECT_EQ(memcmp(r1.getData(), segment.lookup(tid1).getData(), r1.getLen()),0);
    EXPECT_EQ(memcmp(r2.getData(), segment.lookup(tid2).getData(), r2.getLen()),0);
}
