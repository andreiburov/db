#include "gtest/gtest.h"
#include "../../database/dbms.h"
#include "OperatorsHelper.h"

TEST(Print, Simple)
{
    init();

    RecordHelper r1(relation1, "Andrei", sizeof("Andrei"), 21, "Saint-Petersburg", sizeof("Saint-Petersburg"), 12345);
    RecordHelper r2(relation1, "Viktoria", sizeof("Viktoria"), 18, "Kopenhagen", sizeof("Kopenhagen"), 68733);
    RecordHelper r3(relation1, "Grizzy", sizeof("Grizzy"), 30, "Leipzig", sizeof("Leipzig"), 778123);

    segment1.insert(r1.get());
    segment1.insert(r2.get());
    segment1.insert(r3.get());

    TableScan* ts = new TableScan(relation1, segment1);
    //std::stringstream ss;
    Print pr(ts, std::cout);

    pr.open();

    ASSERT_TRUE(pr.next());
//  std::cout << ss.str();
//  ASSERT_STREQ("Andrei21Saint-Petersburg12345", ss.str().c_str());

    ASSERT_TRUE(pr.next());
//  ASSERT_STREQ("Viktoria18Kopenhagen68733", ss.str().c_str());

    ASSERT_TRUE(pr.next());
//  ASSERT_STREQ("Grizzy30Leipzig778123", ss.str().c_str());
    ASSERT_FALSE(pr.next());

    delete ts;
}