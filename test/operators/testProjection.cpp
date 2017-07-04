#include "gtest/gtest.h"
#include "../../database/dbms.h"
#include "OperatorsHelper.h"

TEST(Projection, Simple)
{
    init();

    RecordHelper r1(relation1, "Andrei", sizeof("Andrei"), 21, "Saint-Petersburg", sizeof("Saint-Petersburg"), 12345);
    RecordHelper r2(relation1, "Viktoria", sizeof("Viktoria"), 18, "Kopenhagen", sizeof("Kopenhagen"), 68733);
    RecordHelper r3(relation1, "Grizzy", sizeof("Grizzy"), 30, "Leipzig", sizeof("Leipzig"), 778123);

    segment1.insert(r1.get());
    segment1.insert(r2.get());
    segment1.insert(r3.get());

    Register reg(18);
    TableScan* ts = new TableScan(relation1, segment1);
    Projection pr(ts, {0, 1});

    pr.open();
    std::vector<Register> out;

    ASSERT_TRUE(pr.next());
    out = pr.getOutput();
    ASSERT_STREQ("Andrei", out[0].getString().c_str());
    ASSERT_EQ(21, out[1].getInteger());

    ASSERT_TRUE(pr.next());
    out = pr.getOutput();
    ASSERT_STREQ("Viktoria", out[0].getString().c_str());
    ASSERT_EQ(18, out[1].getInteger());

    ASSERT_TRUE(pr.next());
    out = pr.getOutput();
    ASSERT_STREQ("Grizzy", out[0].getString().c_str());
    ASSERT_EQ(30, out[1].getInteger());

    ASSERT_FALSE(pr.next());

    delete ts;
}