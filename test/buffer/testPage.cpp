#include "gtest/gtest.h"
#include "../../database/dbms.h"

TEST(Page, GetSegment) {
    uint64_t segment0 = 0xffff;
    uint64_t segment1 = 0x1ffff;
    uint64_t segment2 = 0x2ffff;
    uint64_t segment100 = 0x100ffff;
    EXPECT_EQ(0x0, GetSegment(segment0));
    EXPECT_EQ(0x1, GetSegment(segment1));
    EXPECT_EQ(0x2, GetSegment(segment2));
    EXPECT_EQ(0x100, GetSegment(segment100));
}

TEST(Page, GetPageOffset) {
    uint64_t page_offset0 = 0x10000;
    uint64_t page_offset2 = 0xff0002;
    uint64_t page_offset50 = 0x0050;
    uint64_t page_offsetffff = 0xffff;
    EXPECT_EQ(0x0, GetPageOffset(page_offset0));
    EXPECT_EQ(0x2, GetPageOffset(page_offset2));
    EXPECT_EQ(0x50, GetPageOffset(page_offset50));
    EXPECT_EQ(0xffff, GetPageOffset(page_offsetffff));
}

TEST(Page, GetFirstPage) {
    uint64_t segment0 = 0x0;
    uint64_t segment1 = 0x10000;
    uint64_t segment2 = 0x20000;
    uint64_t segment100 = 0x1000000;
    EXPECT_EQ(segment0, GetFirstPage(0x0));
    EXPECT_EQ(segment1, GetFirstPage(0x1));
    EXPECT_EQ(segment2, GetFirstPage(0x2));
    EXPECT_EQ(segment100, GetFirstPage(0x100));
}