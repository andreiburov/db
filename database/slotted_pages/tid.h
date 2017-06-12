#pragma once

#include "../buffer_manager/page_size.h"
#include <stdint.h>
#include <functional>

#define SLOT_SIZE_IN_BITS (64-PAGE_SIZE_IN_BITS)

struct TID {

  uint64_t slot_id : SLOT_SIZE_IN_BITS;
  uint64_t page_id : PAGE_SIZE_IN_BITS;

  TID(uint64_t slot_id, uint64_t page_id) : slot_id(slot_id), page_id(page_id) {}
  TID(uint64_t id) : slot_id(id >> PAGE_SIZE_IN_BITS), page_id(id & PAGE_MASK) {}
  TID() : slot_id(0), page_id(0) {}

  bool operator==(const TID& other) const {
    return slot_id == other.slot_id &&
           page_id == other.page_id;
  }

};

namespace std {
  template<>
  struct hash<TID> {
    size_t operator()(const TID &tid) const {
      return hash<uint64_t>()(tid.page_id)
              ^ (hash<uint64_t>()(tid.slot_id) << 1);
    }
  };

}
