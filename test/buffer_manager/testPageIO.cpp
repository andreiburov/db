#include "gtest/gtest.h"
#include "../../database/buffer/Page.h"
#include "../../database/buffer/PageIO.h"

#include <thread>

int GetFileSize(std::string filename) {
    struct stat stat_buf;
    int rc = stat(filename.c_str(), &stat_buf);
    return rc == 0 ? stat_buf.st_size : -1;
}

uint64_t g_page_id = 0x100;
const char* g_data = "hello";
const int g_data_size = 4;
const int g_thread_count = 10;

void setUp() {
    std::string segment(std::to_string(GetSegment(g_page_id)));
    if (unlink(segment.c_str()) != 0) {
        perror("Could not delete the segment file");
    }
}

TEST(PageIO, WriteAfterEndOfFile) {
    setUp();
    {
        PageIO page_io;
        page_io.writePage(g_page_id, (void*)g_data, strlen(g_data));
    }
    int expected_size = g_page_id*GetPageSize() + strlen(g_data);
    std::string segment(std::to_string(GetSegment(g_page_id)));
    EXPECT_EQ(expected_size, GetFileSize(segment));
}

TEST(PageIO, ReadAfterEndOfFile) {
    setUp();
    char data[g_data_size+1];
    char expected[g_data_size+1];
    data[g_data_size] = '\0'; // strcmp compares null terminated strings
    expected[g_data_size] = '\0';
    unsigned length = g_data_size;
    memset(data, '0', length);
    memset(expected, '0', length);
    {
        PageIO page_io;
        page_io.readPage(g_page_id, (void*)data, length);
    }
    EXPECT_STREQ(expected, data);
    int expected_size = 0;
    std::string segment(std::to_string(GetSegment(g_page_id)));
    EXPECT_EQ(expected_size, GetFileSize(segment));
}

void readWrite(PageIO& page_io, unsigned thread_number) {
    for (int i = 0; i < 1000; i++) {
        uint64_t page_offset = GetPageOffset(rand_r(&thread_number))/g_thread_count + thread_number;
        char data[g_data_size];
        unsigned length = g_data_size;
        page_io.writePage(page_offset, (void*)&thread_number, length);
        page_io.readPage(page_offset, data, length);
        EXPECT_EQ(thread_number, *(unsigned*)data);
    }
}

TEST(PageIO, ReadWriteInSegment) {
    setUp();
    PageIO page_io;
    std::vector<std::thread> threads;

    for (int i = 0; i < g_thread_count; ++i) {
        std::thread t(readWrite, std::ref(page_io), i);
        threads.emplace_back(std::move(t));
    }

    for (int i = 0; i < g_thread_count; ++i) {
        threads[i].join();
    }
}