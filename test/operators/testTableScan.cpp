#include "gtest/gtest.h"
#include "../../database/dbms.h"
#include "OperatorsHelper.h"

TEST(TableScan, ReadWrite)
{
    init();

    RecordHelper r1(relation1, "Andrei", sizeof("Andrei"), 21, "Saint-Petersburg", sizeof("Saint-Petersburg"), 12345);
    RecordHelper r2(relation1, "Viktoria", sizeof("Viktoria"), 18, "Kopenhagen", sizeof("Kopenhagen"), 68733);
    RecordHelper r3(relation1, "Grizzy", sizeof("Grizzy"), 30, "Leipzig", sizeof("Leipzig"), 778123);

    segment1.insert(r1.get());
    segment1.insert(r2.get());
    segment1.insert(r3.get());

    TableScan ts(relation1, segment1);
    ts.open();

    std::vector<Register> out;

    ASSERT_TRUE(ts.next());
    out = ts.getOutput();
    ASSERT_STREQ("Andrei", out[0].getString().c_str());
    ASSERT_EQ(21, out[1].getInteger());
    ASSERT_STREQ("Saint-Petersburg", out[2].getString().c_str());
    ASSERT_EQ(12345, out[3].getInteger());

    ASSERT_TRUE(ts.next());
    out = ts.getOutput();
    ASSERT_STREQ("Viktoria", out[0].getString().c_str());
    ASSERT_EQ(18, out[1].getInteger());
    ASSERT_STREQ("Kopenhagen", out[2].getString().c_str());
    ASSERT_EQ(68733, out[3].getInteger());

    ASSERT_TRUE(ts.next());
    out = ts.getOutput();
    ASSERT_STREQ("Grizzy", out[0].getString().c_str());
    ASSERT_EQ(30, out[1].getInteger());
    ASSERT_STREQ("Leipzig", out[2].getString().c_str());
    ASSERT_EQ(778123, out[3].getInteger());

    ASSERT_FALSE(ts.next());
}