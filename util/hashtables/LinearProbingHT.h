#ifndef DB_LINEARPROBINGHT_H
#define DB_LINEARPROBINGHT_H

#include <cstdint>
#include <cstring>
#include <atomic>

template<typename HashFunction>
class LinearProbingHT {

public:

    // Entry
    struct Entry {
        uint64_t key;
        uint64_t value;
        std::atomic<bool> free;
    };

    Entry* entries_;
    uint64_t size_;
    HashFunction hash_;

    // Constructor
    LinearProbingHT(uint64_t size) : size_(size*2), hash_(HashFunction())
    {
        entries_ = new Entry[size_];
        for (size_t i = 0; i < size_; ++i) {
            entries_[i].key = 0;
            entries_[i].value = 0;
            entries_[i].free = true;
        }
    }

    // Destructor
    ~LinearProbingHT()
    {
        delete[] entries_;
    }

    // Returns the number of hits
    inline uint64_t lookup(uint64_t key)
    {
        uint64_t h = hash_(key) % size_; // calculate hash
        uint64_t hits = 0;
        while (!entries_[h].free) {
            hits += entries_[h].key == key;
            h = (h + 1) % size_;
        }
        return hits;
    }

    inline void insert(uint64_t key, uint64_t value)
    {
        uint64_t h = hash_(key) % size_; // calculate hash
        bool expected = true; // entry should not be occupied
        bool desired = false; // entry.free value in case of success
        do {
            while (!entries_[h].free)
                h = (h + 1) % size_;
        } while (!entries_[h].free.compare_exchange_weak(expected, desired, std::memory_order_seq_cst));

        entries_[h].key = key;
        entries_[h].value = value;
    }
};

#endif //DB_LINEARPROBINGHT_H
