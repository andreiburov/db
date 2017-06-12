#include "gtest/gtest.h"
#include "../../database/dbms.h"

TEST(Manager, WriteReadValue) {
    unsigned expected = 123;
    BufferManager* bm = new BufferManager(1);
    BufferFrame& frame = bm->fixPage(0, true);
    reinterpret_cast<unsigned*>(frame.getData())[0]=expected;
    bm->unfixPage(frame, true);
    delete bm;

    bm = new BufferManager(1);
    frame = bm->fixPage(0, false);
    ASSERT_EQ(expected, reinterpret_cast<unsigned*>(frame.getData())[0]);
    bm->unfixPage(frame, false);
}