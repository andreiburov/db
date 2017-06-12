#include "record.h"

#include <cstring>
#include <cstdlib>

Record::Record(unsigned length, const char* const data) : length_(length) 
{
  data_ = static_cast<char*>(malloc(length_));
  if (data_) {
    memcpy(data_, data, length_);
  }
}

Record &operator=(Record &rhs) = delete;

Record(Record &t) = delete;

Record(Record &&t);

~Record();

const char* getData() const;

unsigned getLength() const;
