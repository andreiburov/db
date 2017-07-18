#ifndef DB_LOCKCHAININGHT_H
#define DB_LOCKCHAININGHT_H

#include <cstdint>
#include <tbb/tbb.h>

template<typename HashFunction>
class LockChainingHT {

public:

    struct Entry {
        uint64_t key;
        uint64_t value;
        Entry* next;
    };

    Entry* entries_;
    Entry* free_entry_;
    Entry** table_;
    tbb::spin_mutex* chain_locks_;
    uint64_t size_;
    HashFunction hash_;

    LockChainingHT(uint64_t size) : size_(size*2), hash_(HashFunction())
    {
        entries_ = new Entry[size];
        free_entry_ = entries_;

        for (size_t i = 0; i < size; ++i) {
            entries_[i].key = 0;
            entries_[i].value = 0;
            entries_[i].next = nullptr;
        }

        table_ = new Entry*[size_];

        for (size_t i = 0; i < size_; ++i) {
            table_[i] = nullptr;
        }

        chain_locks_ = new tbb::spin_mutex[size_];
    }

    ~LockChainingHT()
    {
        delete[] entries_;
        delete[] table_;
        delete[] chain_locks_;
    }

    inline uint64_t lookup(uint64_t key)
    {
        uint64_t h = hash_(key) % size_; // calculate hash
        uint64_t hits = 0;

        Entry* node = table_[h];
        while (node != nullptr) {
            hits += node->key == key;
            node = node->next;
        }
        return hits;
    }

    inline void insert(uint64_t key, uint64_t value)
    {
        Entry* entry = free_entry_++;
        entry->key = key;
        entry->value = value;

        uint64_t h = hash_(key) % size_; // calculate hash
        tbb::spin_mutex::scoped_lock lock;
        lock.acquire(chain_locks_[h]);

        Entry* node = table_[h];
        Entry* next;

        if (node == nullptr) {
            table_[h] = entry;
        } else {
            next = node->next;
            while (next != nullptr) {
                node = next;
                next = next->next;
            }
            node->next = entry;
        }


        lock.release();
    }
};

#endif //DB_LOCKCHAININGHT_H
