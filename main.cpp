#include <iostream>
#include <cstring>
#include <stdint-gcc.h>

#include "database/buffer/PageIO.h"

using namespace std;

int main()
{
    const char* data = "hello";
    PageIO pageIO;
    pageIO.writePage((uint64_t)100U, (void*)data, strlen(data));
    return 0;
}
