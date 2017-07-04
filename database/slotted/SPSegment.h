#ifndef DB_SPSEGMENT_H
#define DB_SPSEGMENT_H

#include "Segment.h"
#include "TID.h"
#include "Record.h"
#include <atomic>

/* SPSegment
 *
 * Operates on slotted pages
 *
 * - TID SPSegment::insert(const Record& r)
 *     Search through the segment’s pages looking for a page with enough space to store r.
 *     TID (tuple identifier) consists of a page offset ID and a slot ID.
 *
 * - bool SPSegment::remove(TID tid)
 *     Deletes the record pointed to by tid and updates the slotted page header accordingly.
 *
 * - Record SPSegment::lookup(TID tid)
 *     Returns the read-only record (Record.hpp) associated with TID tid.
 *
 * - bool SPSegment::update(TID tid, const Record& r)
 *     Updates the record pointed to by tid with the content of record r.
 */

class SPSegment : public Segment {

public:

    // Slotted Page Header
    struct Header {
        // log sequence number for recovery
        uint64_t LSN;
        // number of used slots
        uint16_t slot_count;
        uint16_t first_free_slot;
        // lower end of data
        uint64_t data_start;
        // space that would be available after compactification
        unsigned fragmented_space;
        Header() : slot_count(0), first_free_slot(0), data_start(PAGESIZE),
                   fragmented_space(PAGESIZE-sizeof(Header)) {}
    };

    struct Slot {
        uint64_t offset;
        uint64_t length;

        Slot() : offset(0), length(0) {}

        bool isFree() {
            return offset == 0;
        }

        bool isRedirection() {
            return offset == 1;
        }

        // is currently updated by SPSegment::update
        bool isUpdated() {
            return offset == 2;
        }

        bool isRecord() {
            return offset >= 3;
        }

        TID getRedirection() {
            return TID(length);
        }

        void setRedirection(TID tid) {
            offset = 1;
            length = tid.uint64();
        }

        void setFree() {
            offset = 0;
            length = 0;
        }

        void setUpdated() {
            offset = 2;
        }
    };

public:

    SPSegment(BufferManager& buffer_manager, uint64_t segment_id)
        : Segment(buffer_manager, segment_id) {}

    ~SPSegment() {}

    TID insert(const Record& r);

    bool remove(TID tid);

    Record lookup(TID tid);

    std::vector<Record> lookupRecordsInPage(uint64_t page_id);

    bool update(TID tid, const Record& r);

    // private: exposed for tests

    void compactify(Header* header);

    void insert(const Record& r, Header* header, Slot* slot);

    inline Slot* getFirstSlot(Header* header) {
        return reinterpret_cast<Slot*>(reinterpret_cast<char*>(header)+sizeof(Header));
    }

    inline uint64_t getNumberOfRecords() { return this->size_; }

private:

    void checkOverlaps(Header *header, Slot *slot);
};


#endif //DB_SPSEGMENT_H
