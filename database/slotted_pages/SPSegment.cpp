#include <cassert>
#include <queue>
#include <iostream>
#include "SPSegment.h"
#include "../utils/LockGuards.h"

TID SPSegment::insert(const Record &r) {
    uint64_t page_offset = 0;
    uint16_t slot_id = -1;
    Header* header;

    for (;page_offset < max_page_; ++page_offset)
    {
        FrameGuard guard(buffer_manager_, GetFirstPage(segment_id_)+page_offset, true);
        header = reinterpret_cast<Header*>(guard.frame.getData());

        if (r.getLen()+sizeof(Slot) <= header->fragmented_space) {
            // slot_end after adding a new slot
            uint64_t slot_end = sizeof(Header) + (header->slot_count+1)*sizeof(Slot);
            if (r.getLen() > (header->data_start - slot_end)) {
                compactify(header);
            }

            slot_id = header->first_free_slot;
            if (slot_id < header->slot_count) // free slot found
            {                                 // look for new first free slot
                uint16_t i = header->first_free_slot + 1;
                for (; i < header->slot_count; ++i) {
                    Slot& slot = getFirstSlot(header)[i];
                    if (slot.isFree()) {
                        break;
                    }
                }
                header->first_free_slot = i;
                header->fragmented_space -= r.getLen();
            } else { // insert new slot
                assert(slot_id == header->slot_count);
                header->slot_count++;
                header->first_free_slot = header->slot_count;
                header->fragmented_space -= sizeof(Slot) + r.getLen();
            }
            Slot* slot = new(getFirstSlot(header) + slot_id) Slot();
            insert(r, header, slot);
            return TID(slot_id, page_offset);
        }
    }

    FrameGuard guard(buffer_manager_, GetFirstPage(segment_id_)+page_offset, true);

    // in multithreaded environment max_page_ could have been incremented to this point
    // ergo the page was already created, we should rewrite this part
    if (page_offset == max_page_) // create new page
    {
        max_page_++;
        slot_id = 0;
        Header* header = new(guard.frame.getData()) Header();
        Slot* slot = new(getFirstSlot(header)) Slot();
        header->slot_count++;
        header->first_free_slot++;
        header->fragmented_space -= sizeof(Slot) + r.getLen();
        insert(r, header, slot);
    }

    return TID(slot_id, page_offset);
}

void SPSegment::insert(const Record &r, SPSegment::Header *header, SPSegment::Slot *slot) {
    header->data_start -= r.getLen();
    slot->offset = header->data_start;
    slot->length = r.getLen();

    //checkOverlaps(header, slot);

    memcpy(reinterpret_cast<char*>(header)+slot->offset, r.getData(), r.getLen());
}

bool SPSegment::remove(TID tid) {
    uint64_t page_id = GetFirstPage(segment_id_) + tid.page_offset;
    FrameGuard guard(buffer_manager_, page_id, true);

    Header* header = reinterpret_cast<Header*>(guard.frame.getData());
    Slot& slot = getFirstSlot(header)[tid.slot_id];

    if (slot.isFree()) {
        return false;
    }

    if (tid.slot_id < header->first_free_slot) {
        header->first_free_slot = tid.slot_id;
    }

    if (slot.isRedirection()) {
        TID redirect_tid = slot.getRedirection();
        slot.setFree();
        return remove(redirect_tid);
    } else {
        if (slot.offset == header->data_start) {
            header->data_start += slot.length;
        }
        header->fragmented_space += slot.length;
        slot.setFree();
    }

    return true;
}

Record SPSegment::lookup(TID tid) {
    uint64_t page_id = GetFirstPage(segment_id_) + tid.page_offset;
    FrameGuard guard(buffer_manager_, page_id, false);

    char* data = reinterpret_cast<char*>(guard.frame.getData());
    Slot& slot = reinterpret_cast<Slot*>(data+sizeof(Header))[tid.slot_id];

    if (slot.isRedirection()) {
        return lookup(slot.getRedirection());
    } else {
        return Record(slot.length, data + slot.offset);
    }
}

bool SPSegment::update(TID tid, const Record &r) {
    uint64_t page_id = GetFirstPage(segment_id_) + tid.page_offset;
    FrameGuard guard(buffer_manager_, page_id, true);

    Header* header = reinterpret_cast<Header*>(guard.frame.getData());
    Slot& slot = getFirstSlot(header)[tid.slot_id];

    if (slot.isRecord()) {
        if (r.getLen() <= slot.length) {
            header->fragmented_space += slot.length - r.getLen();
            slot.length = r.getLen();
            memcpy(reinterpret_cast<char*>(header) + slot.offset, r.getData(), r.getLen());
        } else { // new record content doesn't fit
            if (slot.isUpdated()) {
                return false; // we allow only one update at a time
            }
            uint64_t offset = slot.offset;
            slot.setUpdated();
            guard.unlock();

            TID new_tid = insert(r);

            guard.lock();
            if (new_tid.page_offset == tid.page_offset) // new TID in the same page
            {
                Slot& new_slot = getFirstSlot(header)[new_tid.slot_id];
                slot.offset = new_slot.offset;
                slot.length = new_slot.length;
                new_slot.setFree();
                if (new_tid.slot_id < header->first_free_slot) {
                    header->first_free_slot = new_tid.slot_id;
                }
            } else { // add Redirection to the new TID
                slot.setRedirection(new_tid);
            }
        }
    } else if (slot.isRedirection())
    {
        TID rd_tid = slot.getRedirection(); // Redirection TID
        assert(tid != rd_tid);
        guard.unlock(false);

        if (!update(rd_tid, r)) {
            return false;
        }

        FrameGuard rd_guard(buffer_manager_, GetFirstPage(segment_id_) + rd_tid.page_offset, true);
        Header* rd_header = reinterpret_cast<Header*>(rd_guard.frame.getData());
        Slot& rd_slot = getFirstSlot(rd_header)[rd_tid.slot_id];

        if (rd_slot.isRedirection()) // collapse two level Redirection
        {
            guard.lock(rd_guard); // to avoid deadlock
            slot.setRedirection(rd_slot.getRedirection());
            guard.unlock();

            rd_slot.setFree();
            if (rd_tid.slot_id < rd_header->first_free_slot) {
                rd_header->first_free_slot = rd_tid.slot_id;
            }
        }
    } else { // Slot is nor Record, nor Redirection
        return false;
    }

    return true;
}

void SPSegment::compactify(Header* header) {
    struct SlotComparator {
        bool operator() (Slot* const &a, Slot* const &b) {
            return a->offset < b->offset;
        }
    };

    std::priority_queue<Slot*, std::vector<Slot*>, SlotComparator> queue;
    char* data = reinterpret_cast<char*>(header);
    Slot* slots = reinterpret_cast<Slot*>(data+sizeof(Header));

    for (uint16_t slot_id = 0; slot_id < header->slot_count; ++slot_id) {
        Slot& slot = slots[slot_id];

        if (slot.isRecord()) {
            queue.push(&slot);
        }
    }

    uint64_t offset = PAGESIZE;
    while (!queue.empty()) {
        Slot* slot = queue.top();
        queue.pop();

        offset -= slot->length;
        memmove(data+offset, data+slot->offset, slot->length);
        slot->offset = offset;
    }

    header->data_start = offset;
}

void SPSegment::checkOverlaps(SPSegment::Header *header, SPSegment::Slot *slot) {
    uint64_t b = slot->offset;
    uint64_t e = slot->offset + slot->length;
    int i = 0;
    for (Slot* s = getFirstSlot(header); i < header->slot_count; s+=1, ++i)
    {
        if (s != slot && s->isRecord()) {
            if (s->offset < b && s->offset + s->length > b) {
                std::cerr << "Overlapping beginning" << std::endl;
            }
            if (s->offset < e && s->offset + s->length > e) {
                std::cerr << "Overlapping ending" << std::endl;
            }
        }
    }
}
