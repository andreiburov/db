#ifndef DB_TID_H
#define DB_TID_H

#include <cstdint>
#include <functional>

struct TID {
    // sizeof 64 bits
    uint16_t slot_id;
    uint64_t page_offset : 48;

    static const uint64_t SLOT_MASK = 0xFFFF000000000000;
    static const uint64_t PAGE_MASK = 0xFFFFFFFFFFFF;

    TID(uint16_t slot_id, uint64_t page_offset) : slot_id(slot_id), page_offset(page_offset) {}

    TID(uint64_t tid) : slot_id((uint16_t) (tid & SLOT_MASK)), page_offset(tid & PAGE_MASK) {}

    TID() : slot_id(0), page_offset(0) { }

    uint64_t uint64() { return (slot_id<<48)|page_offset; }

    bool operator==(const TID& other) const {
        return slot_id == other.slot_id &&
               page_offset == other.page_offset;
    }
};

namespace std {
    template<>
    struct hash<TID> {
        size_t operator()(const TID &tid) const {
            return (hash<uint64_t>()(tid.page_offset)
                    ^ (hash<uint16_t>()(tid.slot_id) << 1)) >> 1;
        }
    };
}

#endif //DB_TID_H
