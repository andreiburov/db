#include "buffer_manager.h"

#include <cstdlib>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <iostream>

#include "../utils/lockguards.h"

uint64_t GetPageCountInSegment() { return 1 << 16; }

void BufferManager::allocateFramesForSegment(Segment& segment, uint64_t segment_id)
{
  uint64_t page_id = GetFirstPage(segment_id);
  uint64_t page_count = GetPageCountInSegment();

  for (; page_id < page_count; page_id++) {
    unsigned i = page_count_;
    bool loaded = false;
    if (free_pages_count_ != 0) {
      fifo_.push_back(page_id);
      i = free_pages_count_ - 1;
      loaded = true;
      --free_pages_count_;
      segment.readPage(&page_array_[i], GetOffset(page_id));
#ifdef DEBUG
      log_ << "newpage " << page_id << " index " << i << std::endl;
#endif
    }

    BufferFrame frame(loaded, false, i, this, page_id);
    registry_[page_id] = frame;
  }
}

BufferManager::BufferManager(unsigned page_count) 
: page_count_(page_count), free_pages_count_(page_count) 
{
  std::lock_guard<std::mutex> lock(mutex_);
#ifdef DEBUG
  log_.open("log.txt", std::ios::trunc);
#endif

  registry_.reserve(page_count_);
  page_array_ = (page_t*)malloc(page_count_*sizeof(page_t));

  uint64_t size = GetPageCountInSegment();
  int segment_count = page_count_ / size;
  segment_count += (page_count_ % size == 0) ? 0 : 1;

  for (int i = 0; i < segment_count; i++) {
    Segment segment(i, Segment::FILE_MODE::CREATE);
		if (segment.getPageCount() == 0) { 
    	segment.allocate(GetPageCountInSegment());
		}
    allocateFramesForSegment(segment, i);
  }
}

BufferManager::~BufferManager()
{
  std::unique_lock<std::mutex> lock(mutex_);

  for (auto i = fifo_.begin(); i != fifo_.end(); ++i) {
    if (registry_[*i].dirty_) {
      writeBack(registry_[*i].page_id_);
    }
  }

#ifdef DEBUG
  log_.close();
#endif

  free(page_array_);
}

void BufferManager::writeBack(uint64_t page_id)
{
	registry_[page_id].dirty_ = false;
	int i = registry_[page_id].i_;
  Segment segment(GetSegment(page_id), Segment::FILE_MODE::WRITE);
	segment.writePage(&page_array_[i], GetOffset(page_id));	
#ifdef DEBUG
  log_ << "writeback page " << page_id << " index " << i << " page_array_[i] " << ((unsigned*)&page_array_[i])[0] << std::endl;
#endif
}

bool BufferManager::findSlot(uint64_t page_id) 
{
  BufferFrame& frame_in = registry_[page_id];
  {
    int ret = pthread_rwlock_trywrlock(&frame_in.rwlock_); // will be released in the calling method
    if (ret == EBUSY) {
      return false;
    } else if (ret != 0) {
      std::cerr << "error when trying rwlock on page_id" << std::endl;
      return false;
    }
  }

	auto it = fifo_.begin();
  for (it; it != fifo_.end();) {
    BufferFrame& frame_out = registry_[*it];
    RWLockGuard guard(frame_out.rwlock_);
    int ret = guard.trywrlock();
    if (ret != 0 && ret != EBUSY) {
      pthread_rwlock_unlock(&frame_in.rwlock_);
      std::ostringstream oss;
      oss << "error when trying rwlock: " << strerror(errno) << std::endl;
      throw std::runtime_error(oss.str());
    } else if (ret == EBUSY) {
      ++it;
      continue;
    } else {
#ifdef DEBUG
      log_ << "oldpage " << *it << " is_dirty " << (frame_out.dirty_?"true":"false") << " index " << frame_out.i_ 
        << " newpage " << page_id << " index " << frame_in.i_ << " page_array_[i] " << ((unsigned*)&page_array_[frame_out.i_])[0] << std::endl;
#endif
      frame_in.loaded_ = true;
      frame_in.dirty_ = false;
		  frame_in.i_ = frame_out.i_;	
      if (frame_out.dirty_) {
        writeBack(frame_out.page_id_);
      } 
      frame_out.loaded_ = false;
      frame_out.i_ = page_count_;
			fifo_.erase(it);
		  fifo_.push_back(page_id);
			return true;
    }
  }

  pthread_rwlock_unlock(&frame_in.rwlock_);
  return false;
}

BufferFrame& BufferManager::fixPage(uint64_t page_id, bool exclusive)
{
  std::unique_lock<std::mutex> lock(mutex_);
#ifdef DEBUG
  log_ << "page " << page_id << " fix" << std::endl;
#endif

  auto search = registry_.find(page_id);
  if (search != registry_.end()) 
  {
    BufferFrame& frame = search->second;

    if (frame.loaded_) { 
#ifdef DEBUG
      log_ << "page " << page_id << " is_loaded " << (frame.loaded_?"true":"false") << std::endl;
#endif
    } else {
#ifdef DEBUG
      log_ << "page " << page_id << " is_loaded " << (frame.loaded_?"true":"false") << std::endl;
#endif

      try 
      {
        Segment segment(GetSegment(page_id), Segment::FILE_MODE::READ);

        // find an unfixed pageId, to unload from RAM (writeBack if dirty) throws NoSlotException

        cond_.wait(lock, [this, page_id]{ return findSlot(page_id); });
        segment.readPage(&page_array_[frame.i_], GetOffset(page_id));
#ifdef DEBUG
        log_ << "read page " << page_id << " index " << frame.i_ << " page_array_[i] " << ((unsigned*)&page_array_[frame.i_])[0] << std::endl;
#endif
        pthread_rwlock_unlock(&frame.rwlock_);

      } catch (const NoSlotException &e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        abort();
        throw std::runtime_error(e.what());
      } catch (const std::runtime_error& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
        abort();
        throw e;
      }
    }

    lock.unlock();
#ifdef DEBUG
    log_ << "blocking page " << page_id << " index " << frame.i_ << " page_array_[i] " << ((unsigned*)&page_array_[frame.i_])[0] << std::endl;
#endif
    frame.blockingLock(exclusive);
    return frame;

  } else { // frame was not yet created

    try 
    {
      Segment segment(GetSegment(page_id), Segment::FILE_MODE::CREATE);
      segment.allocate(GetPageCountInSegment());
      allocateFramesForSegment(segment, GetSegment(page_id));

      BufferFrame& frame = registry_[page_id];

      cond_.wait(lock, [this, page_id]{ return findSlot(page_id); });
      segment.readPage(&page_array_[frame.i_], GetOffset(page_id));
      pthread_rwlock_unlock(&frame.rwlock_);
#ifdef DEBUG
      log_ << "read page " << page_id << " index " << frame.i_ << " page_array_[i] " << ((unsigned*)&page_array_[frame.i_])[0] << std::endl;
#endif
    } catch (const NoSlotException& e) {
      std::cerr << "Caught exception: " << e.what() << std::endl;
      abort();
      throw std::runtime_error(e.what());
    } catch (const std::runtime_error& e) {
      abort();
      std::cerr << "Caught exception: " << e.what() << std::endl;
      throw e;
    }

    BufferFrame& frame = registry_[page_id];
    lock.unlock();
#ifdef DEBUG
    log_ << "blocking page" << page_id << " index " << frame.i_ << " page_array_[i] " << ((unsigned*)&page_array_[frame.i_])[0] << std::endl;
#endif
    frame.blockingLock(exclusive);
    return frame;
  }
}

void BufferManager::unfixPage(BufferFrame& frame, bool is_dirty)
{
  std::lock_guard<std::mutex> lock(mutex_);

  frame.dirty_ |= is_dirty;
#ifdef DEBUG
  log_ << "unfix page " << frame.page_id_ << " is_dirty " << (frame.dirty_?"true":"false") << " index " << frame.i_ 
    << " page_array_[i] " << ((unsigned*)&page_array_[frame.i_])[0] << std::endl;
#endif
  pthread_rwlock_unlock(&frame.rwlock_);
  cond_.notify_one();
}

//#ifdef DEBUG
//void BufferManager::print_fifo() {
//  for (: fifo_) {
//    std::cout << " " << e;
//  }
//  std::cout << std::endl;
//}
//#endif
