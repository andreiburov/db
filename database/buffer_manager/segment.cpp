#include "segment.h"

#include <stdexcept>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sstream>
#include <cstring>

Segment::Segment(uint64_t segment_id, FILE_MODE mode)
{
  int flags;
  name_ = std::to_string(segment_id).append(".seg");

  switch (mode) 
  {
    case READ:
    flags = O_RDONLY;
    break;
    
    case WRITE:
    flags = O_WRONLY;
    break;
    
    case CREATE:
    flags = O_RDWR|O_CREAT;
    break;

    default:
    std::ostringstream oss;
    oss << "the mode of openning the file '" << name_ << "' is not supported" << std::endl;
    throw std::runtime_error(oss.str());
  }

  if ((fd_ = open(name_.c_str(), flags, S_IRUSR|S_IWUSR)) < 0) {
    std::ostringstream oss;
    oss << "cannot open file '" << name_ << "': " << strerror(errno) << std::endl;
    throw std::runtime_error(oss.str());
  }
}

Segment::~Segment() 
{
  close(fd_);
}

void Segment::writePage(page_t* page, uint64_t offset) 
{
	if (pwrite64(fd_, page, sizeof(page_t), offset*sizeof(page_t)) < 0) {
    std::ostringstream oss;
    oss << "cannot write page at " << page << " to file '" << name_ << "' with offset " << offset << ": " << strerror(errno) << std::endl;
    throw std::runtime_error(oss.str());
  }
}

void Segment::readPage(page_t* page, uint64_t offset) 
{
	if (pread64(fd_, page, sizeof(page_t), offset*sizeof(page_t)) < 0) {
    std::ostringstream oss;
    oss << "cannot read page to " << page << " to file '" << name_ << "' with offset " << offset << ": " << strerror(errno) << std::endl;
    throw std::runtime_error(oss.str());
  }
}

void Segment::allocate(uint64_t page_count) 
{
  if (posix_fallocate(fd_, 0, page_count*sizeof(page_t)) != 0) {
    std::ostringstream oss;
    oss << "cannot allocate " << page_count*sizeof(page_t) << " bytes to file '" << name_ << "': " << strerror(errno) << std::endl;
    throw std::runtime_error(oss.str());
  }

  if (ftruncate(fd_, page_count*sizeof(page_t)) != 0) {
    std::ostringstream oss;
    oss << "cannot truncate " << page_count*sizeof(page_t) << " bytes to file '" << name_ << "': " << strerror(errno) << std::endl;
    throw std::runtime_error(oss.str());
  }
}

uint64_t Segment::getPageCount()
{
	struct stat st;
  stat(name_.c_str() , &st);
  int size = st.st_size; 
	return size/sizeof(page_t);
}
