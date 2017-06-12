#include "../../database/buffer/BufferManager.h"

#include <iostream>
#include <vector>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <pthread.h>
#include <fstream>
#include <string>

using namespace std;

BufferManager* bm;
unsigned pagesOnDisk;
unsigned pagesInRAM;
unsigned threadCount;
unsigned* threadSeed;
volatile bool stop=false;

std::vector<unsigned> disk;

unsigned randomPage(unsigned threadNum) {
   // pseudo-gaussian, causes skewed access pattern
   unsigned page=0;
   for (unsigned  i=0; i<20; i++)
      page+=rand_r(&threadSeed[threadNum])%pagesOnDisk;
   return page/20;
}

static void* scan(void *arg)
{
#ifdef LOGGING
   std::ofstream file;
   file.open("scn.txt", std::ios::trunc);
#endif
   // scan all pages and check if the counters are not decreasing
   vector<unsigned> counters(pagesOnDisk, 0);

   while (!stop) {
      unsigned start = random()%(pagesOnDisk-10);
      for (unsigned page=start; page<start+10; page++) {
#ifdef LOGGING
      file << "page " << page << " fix" << std::endl;
#endif
         BufferFrame& bf = bm->fixPage(page, false);
         unsigned newcount = reinterpret_cast<unsigned*>(bf.getData())[0];
#ifdef LOGGING
         file << "page " << page << " index " << bf.i_ << " counters[page] " << counters[page] << " newcount " << newcount << std::endl;
#endif
         assert(counters[page]<=newcount);
         /*if(counters[page]<=newcount) {
             exit(123);
         }*/
         counters[page]=newcount;
#ifdef LOGGING
      file << "page " << page << " unfix" << std::endl;
#endif
         bm->unfixPage(bf, false);
      }
   }

#ifdef LOGGING
   file.close();
#endif

   return NULL;
}

static void* readWrite(void *arg) 
{
   uintptr_t threadNum = reinterpret_cast<uintptr_t>(arg);
   vector<unsigned> counters(pagesOnDisk, 0);
#ifdef LOGGING
   std::ofstream file;
   file.open(std::to_string(threadNum).append(".txt").c_str(), std::ios::trunc);
#endif
   // read or write random pages    

   uintptr_t count = 0;
   for (unsigned i=0; i<100000/threadCount; i++) {
      bool isWrite = rand_r(&threadSeed[threadNum])%128<10;
      unsigned page = randomPage(threadNum);
#ifdef LOGGING
      file << "page " << page << " fix" << std::endl;
#endif
      BufferFrame& bf = bm->fixPage(page, isWrite);

      if (isWrite) {
         count++;
         counters[page]++;
         disk[page]++;
         reinterpret_cast<unsigned*>(bf.getData())[0]++;
      }

      assert(disk[page] == reinterpret_cast<unsigned*>(bf.getData())[0]);
#ifdef LOGGING
      file << "page " << page << " index " << bf.i_ << " bf.getData() " << reinterpret_cast<unsigned*>(bf.getData())[0] << std::endl;
      file << "page " << page << " unfix\n";
#endif
      bm->unfixPage(bf, isWrite);
   }
#ifdef LOGGING
   file.close();
#endif

   return reinterpret_cast<void*>(count);
}

int main(int argc, char** argv) {
   if (argc==4) {
      pagesOnDisk = atoi(argv[1]);
      pagesInRAM = atoi(argv[2]);
      threadCount = atoi(argv[3]);
   } else {
      cerr << "usage: " << argv[0] << " <pagesOnDisk> <pagesInRAM> <threads>" << endl;
      exit(1);
   }

   disk.resize(pagesOnDisk);
   threadSeed = new unsigned[threadCount];
   for (unsigned i=0; i<threadCount; i++)
      threadSeed[i] = i*97134;

   bm = new BufferManager(pagesInRAM);

   vector<pthread_t> threads(threadCount);
   pthread_attr_t pattr;
   pthread_attr_init(&pattr);

   // set all counters to 0
   for (unsigned i=0; i<pagesOnDisk; i++) {
      BufferFrame& bf = bm->fixPage(i, true);
      reinterpret_cast<unsigned*>(bf.getData())[0]=0;
      bm->unfixPage(bf, true);
   }

   // start scan thread
   pthread_t scanThread;
   pthread_create(&scanThread, &pattr, scan, NULL);

   // start read/write threads
   for (unsigned i=0; i<threadCount; i++)
      pthread_create(&threads[i], &pattr, readWrite, reinterpret_cast<void*>(i));

   // wait for read/write threads
   unsigned totalCount = 0;
   for (unsigned i=0; i<threadCount; i++) {
      void *ret;
      pthread_join(threads[i], &ret);
      totalCount+=reinterpret_cast<uintptr_t>(ret);
   }

   // wait for scan thread
   stop=true;
   pthread_join(scanThread, NULL);

   // restart buffer manager
   delete bm;
   bm = new BufferManager(pagesInRAM);
   
   // check counter
   unsigned totalCountOnDisk = 0;
   for (unsigned i=0; i<pagesOnDisk; i++) {
      BufferFrame& bf = bm->fixPage(i,false);
      totalCountOnDisk+=reinterpret_cast<unsigned*>(bf.getData())[0];
      bm->unfixPage(bf, false);
   }
   if (totalCount==totalCountOnDisk) {
      cout << "test successful" << endl;
      delete bm;
      return 0;
   } else {
      cerr << "error: expected " << totalCount << " but got " << totalCountOnDisk << endl;
      delete bm;
      return 1;
   }
}
