#include <thread>
#include "gtest/gtest.h"
#include "../../database/dbms.h"

const char* str1 = "Hello World!";
const char* str2 = "X";
const char* str3 = "Why are you looking at me? Go away!";
const char* str4 = "Property is mince?";

const int record_count = 4;

Record r[record_count] = {Record(strlen(str1), str1), Record(strlen(str2), str2),
            Record(strlen(str3), str3), Record(strlen(str4), str4)};

TEST(SPSegment, InsertLookup) {
    BufferManager bm(2);
    SPSegment segment(bm, 1);
    TID tid1 = segment.insert(r[0]);
    TID tid2 = segment.insert(r[1]);
    
    EXPECT_EQ(memcmp(r[0].getData(), segment.lookup(tid1).getData(), r[0].getLen()),0);
    EXPECT_EQ(memcmp(r[0].getData(), segment.lookup(tid2).getData(), r[0].getLen()),0);
}

TEST(SPSegment, Compactify) {
    BufferManager bm(30);
    SPSegment segment(bm, 1);

    uint64_t offset = PAGESIZE;
    uint64_t expected = PAGESIZE;
    unsigned max_pad = 100;
    FrameGuard guard(bm, GetFirstPage(1), true);
    char* data = reinterpret_cast<char*>(guard.frame.getData());
    SPSegment::Header* header = new (data) SPSegment::Header();
    header->slot_count = record_count;

    for (int i = 0; i < record_count; ++i) {
        header->data_start -= rand()%max_pad;
        SPSegment::Slot* slot = new(segment.getFirstSlot(header)+ i) SPSegment::Slot();
        segment.insert(r[i], header, slot);
        expected -= r[i].getLen();
    }

    segment.compactify(header);
    ASSERT_EQ(expected, header->data_start);

    for (int i = 0; i < record_count; ++i) {
        EXPECT_EQ(memcmp(r[i].getData(),
                         reinterpret_cast<char*>(data) + segment.getFirstSlot(header)[i].offset,
                         r[i].getLen()), 0);
    }
}

class Random64 {
    uint64_t state;
public:
    explicit Random64(uint64_t seed=time(NULL)) : state(seed) {}
    uint64_t next() {
        state^=(state<<13); state^=(state>>7); return (state^=(state<<17));
    }
};

void updater_one(SPSegment& segment, TID& tid) {
    Random64 rdm;
    for (int i = 0; i < 1000; i++) {
        segment.update(tid, r[rdm.next()%record_count]);
    }
}

TEST(SPSegment, MultithreadedUpdateOneRecord) {
    BufferManager bm(30);
    SPSegment segment(bm, 1);

    TID tid = segment.insert(r[0]);

    std::thread t1(updater_one, std::ref(segment), std::ref(tid));
    std::thread t2(updater_one, std::ref(segment), std::ref(tid));
    std::thread t3(updater_one, std::ref(segment), std::ref(tid));
    t1.join();
    t2.join();
    t3.join();

    bool found = false;
    char buf[200];
    for (int i = 0; i < record_count; ++i) {
       if (memcmp(r[i].getData(), segment.lookup(tid).getData(), r[i].getLen()) == 0)
       {
           found = true;
           memcpy(buf, r[i].getData(), r[i].getLen());
           buf[r[i].getLen()] = '\0';
           std::cout << std::endl << "Found record: " << buf << std::endl;
       }
    }
    ASSERT_TRUE(found);
}

void updater_two(SPSegment& segment, TID* tid) {
    Random64 rdm;
    for (int i = 0; i < 10000; i++) {
        segment.update(tid[rdm.next()%2], r[rdm.next()%record_count]);
    }
}

TEST(SPSegment, MultithreadedUpdateTwoRecords) {
    BufferManager bm(30);
    SPSegment segment(bm, 1);

    TID tid[2];
    tid[0] = segment.insert(r[0]);
    tid[1] = segment.insert(r[2]);

    std::thread t1(updater_two, std::ref(segment), tid);
    std::thread t2(updater_two, std::ref(segment), tid);
    std::thread t3(updater_two, std::ref(segment), tid);
    t1.join();
    t2.join();
    t3.join();

    bool found = false;
    char buf[200];
    for (int i = 0; i < record_count; ++i) {
        if (memcmp(r[i].getData(), segment.lookup(tid[0]).getData(), r[i].getLen()) == 0)
        {
            found = true;
            memcpy(buf, r[i].getData(), r[i].getLen());
            buf[r[i].getLen()] = '\0';
            std::cout << std::endl << "Found record: " << buf << std::endl;
        }

        if (memcmp(r[i].getData(), segment.lookup(tid[1]).getData(), r[i].getLen()) == 0)
        {
            found = true;
            memcpy(buf, r[i].getData(), r[i].getLen());
            buf[r[i].getLen()] = '\0';
            std::cout << std::endl << "Found record: " << buf << std::endl;
        }
    }
    ASSERT_TRUE(found);
}