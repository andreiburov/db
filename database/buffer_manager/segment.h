#pragma once

#include "page_size.h"

#include <string>

class Segment {

  public:

  enum FILE_MODE { READ, WRITE, CREATE };

  private:
  
  int fd_;
  std::string name_;

  public:

  Segment(uint64_t segment_id, FILE_MODE mode);

  ~Segment();

  void writePage(page_t* page, uint64_t offset);

  void readPage(page_t* page, uint64_t offset);

  void allocate(uint64_t page_count); 

  uint64_t getPageCount();
};
