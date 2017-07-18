#ifndef DB_LOCKFREECHAININGHT_H
#define DB_LOCKFREECHAININGHT_H

#include <cstdint>
#include <atomic>

template<typename HashFunction>
class LockFreeChainingHT {

public:

    struct Entry {
        uint64_t key;
        uint64_t value;
        std::atomic<Entry*> next;
    };

    Entry* entries_;
    Entry* free_entry_;
    std::atomic<Entry*>* table_;
    uint64_t size_;
    HashFunction hash_;

    LockFreeChainingHT(uint64_t size) : size_(size*2), hash_(HashFunction())
    {
        entries_ = new Entry[size];
        free_entry_ = entries_;

        for (size_t i = 0; i < size; ++i) {
            entries_[i].key = 0;
            entries_[i].value = 0;
            entries_[i].next = nullptr;
        }

        table_ = new std::atomic<Entry*>[size_];

        for (size_t i = 0; i < size_; ++i) {
            table_[i] = nullptr;
        }
    }

    ~LockFreeChainingHT()
    {
        delete[] entries_;
        delete[] table_;
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

    inline void insert(uint64_t key, uint64_t value) {

        Entry* entry = free_entry_++;
        entry->key = key;
        entry->value = value;

        Entry* expected = nullptr;
        uint64_t h = hash_(key) % size_; // calculate hash

        do {
            if (table_[h] != nullptr)
                break;
        } while (!table_[h].compare_exchange_weak(expected, entry, std::memory_order_seq_cst));

        std::atomic<Entry*>& node = table_[h];

        do {
            while (node.load()->next != nullptr)
                node.store(node.load()->next);
        } while (!node.compare_exchange_weak(expected, entry, std::memory_order_seq_cst));
    }
};

#endif //DB_LOCKFREECHAININGHT_H
