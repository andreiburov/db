#pragma once

class Record {

	private:

  unsigned length_;
  char* data_;

	public:

  Record(unsigned length, const char* const data);

  Record &operator=(Record &rhs) = delete;

  Record(Record &t) = delete;

  Record(Record &&t);

  ~Record();

  const char* getData() const;

  unsigned getLength() const;
};

