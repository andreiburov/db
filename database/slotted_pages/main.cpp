#include "tid.h"
#include <iostream>

int main(int argc, char** argv) {

  TID tid(0xfffffffffffff, 0xfffff);
  std::cout << "page_id " << tid.slot_id << std::endl;
  std::cout << "sizeof tid " << sizeof(TID) << std::endl;
  std::cout << "sizeof uint64 " << sizeof(uint64_t) << std::endl;
  return 0;
}
