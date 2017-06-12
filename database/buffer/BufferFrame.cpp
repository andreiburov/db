#include "BufferFrame.h"
#include "BufferManager.h"

void* BufferFrame::getData()
{
    return buffer_manager_->getPage(i_);
}

bool BufferFrame::blockingLock(bool exclusive)
{
    int ret = (exclusive ? pthread_rwlock_wrlock : pthread_rwlock_rdlock)(&rwlock_);
    if (ret == 0) {
        return true;
    }
    return false;
}

bool BufferFrame::tryLock(bool exclusive)
{
    int ret = (exclusive ? pthread_rwlock_trywrlock : pthread_rwlock_tryrdlock)(&rwlock_);
    if (ret == 0) {
        return true;
    }
    return false;
}

void BufferFrame::unlock() {
    pthread_rwlock_unlock(&rwlock_);
}
