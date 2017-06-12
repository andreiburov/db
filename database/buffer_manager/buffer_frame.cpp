#include "buffer_frame.h"
#include "buffer_manager.h"

void* BufferFrame::getData()
{
  return (void*)buffer_manager_->getPage(i_);
}

void BufferFrame::blockingLock(bool exclusive)
{
  if (exclusive) {
    pthread_rwlock_wrlock(&rwlock_);
  } else {
    pthread_rwlock_rdlock(&rwlock_);
  }
}
