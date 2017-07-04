#include "gtest/gtest.h"
#include "../../database/dbms.h"
#include "OperatorsHelper.h"

TEST(HashInnerJoin, Simple) {
    init();

    RecordHelper r1(relation1, "Andrei", sizeof("Andrei"), 21, "Saint-Petersburg", sizeof("Saint-Petersburg"), 12345);
    RecordHelper r2(relation1, "Viktoria", sizeof("Viktoria"), 18, "Kopenhagen", sizeof("Kopenhagen"), 68733);
    RecordHelper r3(relation1, "Grizzy", sizeof("Grizzy"), 30, "Leipzig", sizeof("Leipzig"), 778123);

    segment1.insert(r1.get());
    segment1.insert(r2.get());
    segment1.insert(r3.get());

    RecordHelper r4(relation2, "Grizzy", sizeof("Grizzy"), 40, "Social worker", sizeof("Social worker"), 50);
    RecordHelper r5(relation2, "Andrej", sizeof("Andrej"), 20, "Actor", sizeof("Actor"), 20);
    RecordHelper r6(relation2, "Viktoria", sizeof("Viktoria"), 20, "Musician", sizeof("Musician"), 20);

    segment2.insert(r4.get());
    segment2.insert(r5.get());
    segment2.insert(r6.get());

    TableScan* t1 = new TableScan(relation1, segment1);
    TableScan* t2 = new TableScan(relation2, segment2);
    HashInnerJoin inner_join(t1, t2, 0, 0); // inner_join on name

    inner_join.open();

    std::vector<Register> out;

    ASSERT_TRUE(inner_join.next());
    out = inner_join.getOutput();
    ASSERT_STREQ("Grizzy", out[0].getString().c_str());
    ASSERT_EQ(30, out[1].getInteger());
    ASSERT_STREQ("Leipzig", out[2].getString().c_str());
    ASSERT_EQ(778123, out[3].getInteger());
    ASSERT_STREQ("Grizzy", out[4].getString().c_str());
    ASSERT_EQ(40, out[5].getInteger());
    ASSERT_STREQ("Social worker", out[6].getString().c_str());
    ASSERT_EQ(50, out[7].getInteger());

    ASSERT_TRUE(inner_join.next());
    out = inner_join.getOutput();
    ASSERT_STREQ("Viktoria", out[0].getString().c_str());
    ASSERT_EQ(18, out[1].getInteger());
    ASSERT_STREQ("Kopenhagen", out[2].getString().c_str());
    ASSERT_EQ(68733, out[3].getInteger());
    ASSERT_STREQ("Viktoria", out[4].getString().c_str());
    ASSERT_EQ(20, out[5].getInteger());
    ASSERT_STREQ("Musician", out[6].getString().c_str());
    ASSERT_EQ(20, out[7].getInteger());

    ASSERT_FALSE(inner_join.next());
}