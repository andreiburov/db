#pragma once

#include "page_size.h"
#include "buffer_frame.h"
#include "segment.h"
#include "no_slot_exception.h"
#include "list.h"

#include <unordered_map>
//#include <list>
#include <mutex>
#include <condition_variable>
#include <fstream>

class BufferManager {

  private:

  page_t* page_array_;
  std::unordered_map<uint64_t, BufferFrame> registry_;
  //std::list<uint64_t> fifo_;
  List<uint64_t> fifo_;
  std::mutex mutex_;
  std::condition_variable cond_;
  unsigned page_count_;
  unsigned free_pages_count_;
#ifdef DEBUG
  std::ofstream log_;
#endif

  void allocateFramesForSegment(Segment& segment, uint64_t segment_id);
  bool findSlot(uint64_t page_id);
  void writeBack(uint64_t page_id);
//#ifdef DEBUG
//  void print_fifo();
//#endif

  public:

  BufferManager(unsigned page_count);
  ~BufferManager();
  
  BufferFrame& fixPage(uint64_t page_id, bool exclusive);
  void unfixPage(BufferFrame& frame, bool is_dirty);

  inline page_t* getPage(unsigned i) const {
    return &page_array_[i];
  }
};
