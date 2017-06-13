#ifndef DB_RECORD_H
#define DB_RECORD_H

#include <cstring>
#include <cstdlib>

// A simple Record implementation
class Record {
    unsigned len;
    char* data;

public:
    // Assignment Operator: deleted
    Record& operator=(Record& rhs) = delete;
    // Copy Constructor: deleted
    Record(Record& t) = delete;
    // Move Constructor
    Record(Record&& t);
    // Constructor
    Record(unsigned len, const char* const ptr);
    // Destructor
    ~Record();
    // Get pointer to data
    const char* getData() const;
    // Get data size in bytes
    unsigned getLen() const;
};

inline Record::Record(Record&& t) : len(t.len), data(t.data) {
    t.data = nullptr;
    t.len = 0;
}

inline Record::Record(unsigned len, const char* const ptr) : len(len) {
    data = static_cast<char*>(malloc(len));
    if (data)
        memcpy(data, ptr, len);
}

inline const char* Record::getData() const {
    return data;
}

inline unsigned Record::getLen() const {
    return len;
}

inline Record::~Record() {
    free(data);
}

#endif //DB_RECORD_H
