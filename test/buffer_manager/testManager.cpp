#include "gtest/gtest.h"
#include "../../database/buffer/BufferManager.h"

TEST(Manager, CheckZero) {
    BufferManager bm(1);
    BufferFrame& frame = bm.fixPage(0, true);
    ASSERT_EQ('\0', reinterpret_cast<char*>(frame.getData())[0]);
    bm.unfixPage(frame, false);
}

TEST(Manager, CheckFile) {
    BufferManager bm(10);
    BufferFrame& frame = bm.fixPage(40, true);
}