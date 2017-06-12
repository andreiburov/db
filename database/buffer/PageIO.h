#ifndef DB_PAGEIO_H
#define DB_PAGEIO_H

#include <cstdint>
#include <unordered_map>
#include <mutex>

class PageIO {
    std::unordered_map<uint64_t, int> segments_;
    std::mutex mutex_;

public:
    ~PageIO();
    void writePage(uint64_t page_id, void* data, unsigned length);
    void readPage(uint64_t page_id, void* data, unsigned length);

private:
    void writePage(int fd, void *data, unsigned length, uint64_t offset);
    void readPage(int fd, void *data, unsigned length, uint64_t offset);
};


#endif //DB_PAGEIO_H
