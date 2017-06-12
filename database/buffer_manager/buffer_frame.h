#pragma once

#include "page_size.h"

class BufferManager;

class BufferFrame {

  public:

  bool loaded_;
  bool dirty_;
  unsigned i_; // offset in page array
  BufferManager* buffer_manager_;
  uint64_t page_id_;
  pthread_rwlock_t rwlock_;

  public:
  
  BufferFrame() 
  {
    pthread_rwlock_init(&rwlock_, NULL);
  }

  BufferFrame(bool loaded, bool dirty, unsigned i, BufferManager* buffer_manager, uint64_t page_id) 
    : loaded_(loaded), dirty_(dirty), i_(i), buffer_manager_(buffer_manager), page_id_(page_id)
  {
    pthread_rwlock_init(&rwlock_, NULL);
  }

  ~BufferFrame()
  {
    pthread_rwlock_destroy(&rwlock_);
  }

  void* getData();

  private:

  void blockingLock(bool exclusive);

  friend BufferManager;
};
