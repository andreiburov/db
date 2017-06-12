#include "PageIO.h"
#include "Page.h"

#include <unistd.h>
#include <fcntl.h>
#include <iostream>

PageIO::~PageIO() {
    for (auto it = segments_.cbegin(); it != segments_.cend();) {
        close(it->second);
        it = segments_.erase(it);
    }
}

void PageIO::writePage(uint64_t page_id, void *data, unsigned length) {
    int fd;
    uint64_t segment_id = GetSegment(page_id);
    uint64_t page_offset = GetPageOffset(page_id);
    if (segments_.find(segment_id) != segments_.end()) {
        fd = segments_[segment_id];
    } else {
        std::lock_guard<std::mutex> lock(mutex_);
        fd = open(std::to_string(segment_id).c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
        segments_[segment_id] = fd;
    }
    writePage(fd, data, length, GetFileOffset(page_offset));
}

void PageIO::writePage(int fd, void *data, unsigned length, uint64_t offset) {
    size_t written_bytes = 0;
    while (written_bytes < length) {
        written_bytes += pwrite64(fd, (char*) data + written_bytes, length - written_bytes,
                                offset + written_bytes);
    }
    if (written_bytes > length) {
        perror("Too many bytes were written!");
    }
}

void PageIO::readPage(uint64_t page_id, void *data, unsigned length) {
    int fd;
    uint64_t segment_id = GetSegment(page_id);
    uint64_t page_offset = GetPageOffset(page_id);
    if (segments_.find(segment_id) != segments_.end()) {
        fd = segments_[segment_id];
    } else {
        std::lock_guard<std::mutex> lock(mutex_);
        fd = open(std::to_string(segment_id).c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
        segments_[segment_id] = fd;
    }
    readPage(fd, data, length, GetFileOffset(page_offset));
}

void PageIO::readPage(int fd, void *data, unsigned length, uint64_t offset) {
    size_t read_bytes = 0;
    while (read_bytes < length) {
        size_t bytes = pread(fd, ((char*)data) + read_bytes, length - read_bytes, offset + read_bytes);
        if (bytes == 0) { // end of the file
            break;
        } else if (bytes == -1) { // error
            perror("Error when reading");
            std::cerr << "data "  << data << " length "  << length << " read_bytes " << read_bytes << "\n";
        } else {
            read_bytes += bytes;
        }
    }
    if (read_bytes > length) {
        perror("Too many bytes were read");
    }
}
