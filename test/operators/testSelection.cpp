#include "gtest/gtest.h"
#include "../../database/dbms.h"
#include "OperatorsHelper.h"

TEST(Selection, Simple)
{
    init();

    RecordHelper r1(relation1, "Andrei", sizeof("Andrei"), 18, "Saint-Petersburg", sizeof("Saint-Petersburg"), 12345);
    RecordHelper r2(relation1, "Viktoria", sizeof("Viktoria"), 18, "Kopenhagen", sizeof("Kopenhagen"), 68733);
    RecordHelper r3(relation1, "Grizzy", sizeof("Grizzy"), 30, "Leipzig", sizeof("Leipzig"), 778123);

    segment1.insert(r1.get());
    segment1.insert(r2.get());
    segment1.insert(r3.get());

    Register reg(18);
    TableScan* ts = new TableScan(relation1, segment1);
    Selection sl(ts, 1, reg);

    sl.open();
    std::vector<Register> out;

    ASSERT_TRUE(sl.next());
    out = sl.getOutput();
    ASSERT_STREQ("Andrei", out[0].getString().c_str());
    ASSERT_EQ(18, out[1].getInteger());
    ASSERT_STREQ("Saint-Petersburg", out[2].getString().c_str());
    ASSERT_EQ(12345, out[3].getInteger());

    ASSERT_TRUE(sl.next());
    out = sl.getOutput();
    ASSERT_STREQ("Viktoria", out[0].getString().c_str());
    ASSERT_EQ(18, out[1].getInteger());
    ASSERT_STREQ("Kopenhagen", out[2].getString().c_str());
    ASSERT_EQ(68733, out[3].getInteger());

    ASSERT_FALSE(sl.next());

    delete ts;
}