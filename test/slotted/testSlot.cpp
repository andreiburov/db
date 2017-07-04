#include "gtest/gtest.h"
#include "../../database/dbms.h"

TEST(Slot, GetRedirect) {
    SPSegment::Slot slot;
    slot.offset = 1;
    slot.length = 0x100000000005d;

    ASSERT_EQ(TID(1, 93), slot.getRedirection());
}