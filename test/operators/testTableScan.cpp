#include "gtest/gtest.h"
#include "../../database/dbms.h"

struct RecordHelper {
    char payload[1024];
    unsigned len;

    RecordHelper(Schema::Relation relation,
                 const char* name, unsigned name_len,
                 DB_INT age,
                 const char* city, unsigned city_len,
                 DB_INT number)
    {
        char* p = payload;
        void* v[] = {const_cast<char*>(name), &age, const_cast<char*>(city), &number};
        unsigned i = 0;

        for (auto& attr : relation.attributes) {
            memset(p, '\0', attr.length);
            memcpy(p, v[i++], attr.length);
            p += attr.length;
        }

        len = p - payload;
    }

    Record get() {
        return Record(len, payload);
    }
};

TEST(TableScan, ReadWrite)
{
    BufferManager bm(100);
    SPSegment segment(bm, 1);
    Schema::Relation relation("Profile");
    Schema::Relation::Attribute a1("name", Type::String, 20);
    Schema::Relation::Attribute a2("age", Type::Integer, sizeof(DB_INT));
    Schema::Relation::Attribute a3("city", Type::String, 25);
    Schema::Relation::Attribute a4("number", Type::Integer, sizeof(DB_INT));
    relation.addAttribute(a1);
    relation.addAttribute(a2);
    relation.addAttribute(a3);
    relation.addAttribute(a4);

    RecordHelper r1(relation, "Andrei", sizeof("Andrei"), 21, "Saint-Petersburg", sizeof("Saint-Petersburg"), 12345);
    RecordHelper r2(relation, "Viktoria", sizeof("Viktoria"), 18, "Kopenhagen", sizeof("Kopenhagen"), 68733);
    RecordHelper r3(relation, "Grizzy", sizeof("Grizzy"), 30, "Leipzig", sizeof("Leipzig"), 778123);

    segment.insert(r1.get());
    segment.insert(r2.get());
    segment.insert(r3.get());

    TableScan ts(relation, segment);
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