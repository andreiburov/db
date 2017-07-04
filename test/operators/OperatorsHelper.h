#ifndef DB_OPERATORSHELPER_H
#define DB_OPERATORSHELPER_H

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

BufferManager bm(100);
SPSegment segment1(bm, 1);
SPSegment segment2(bm, 2);
Schema::Relation relation1("Profile1");
Schema::Relation relation2("Profile2");
Schema::Relation::Attribute a11("name", Type::String, 20);
Schema::Relation::Attribute a12("age", Type::Integer, sizeof(DB_INT));
Schema::Relation::Attribute a13("city", Type::String, 25);
Schema::Relation::Attribute a14("number", Type::Integer, sizeof(DB_INT));

Schema::Relation::Attribute a21("name", Type::String, 20);
Schema::Relation::Attribute a22("hours", Type::Integer, sizeof(DB_INT));
Schema::Relation::Attribute a23("occupation", Type::String, 25);
Schema::Relation::Attribute a24("salary", Type::Integer, sizeof(DB_INT));

void init() {
    relation1.addAttribute(a11);
    relation1.addAttribute(a12);
    relation1.addAttribute(a13);
    relation1.addAttribute(a14);

    relation2.addAttribute(a21);
    relation2.addAttribute(a22);
    relation2.addAttribute(a23);
    relation2.addAttribute(a24);
}

#endif //DB_OPERATORSHELPER_H
