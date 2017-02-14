#ifndef STATSALLOC_H
#define STATSALLOC_H

#include <cassert>
#include <functional>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <new>
#include <mutex>
#include <sys/mman.h>

using namespace std;

class Header {
 public:
  size_t requestedSize;  // how big was this object request
  size_t allocatedSize; // how much space was allocated for it
  // prev and next objects, whether allocated or freed.
  Header * prevObject;
  Header * nextObject;
};

template <class SourceHeap>
class StatsAlloc : public SourceHeap {
public:
  StatsAlloc()
    : allocatedObjects (nullptr),
      requested (0),
      allocated (0),
      maxRequested (0),
      maxAllocated (0)
  {
    for (auto& f : freedObjects) {
      f = nullptr;
    }
  }

  ~StatsAlloc()
  {
  }
  
  enum { Alignment = 16 };
  
  void * malloc(size_t sz);
  void free(void * ptr);
  size_t getSize(void * p) const;

  // number of bytes currently allocated  
  size_t bytesAllocated() const;
  
  // max number of bytes allocated  
  size_t maxBytesAllocated() const;

  // number of bytes *requested*
  // (e.g., malloc(4) might result in an allocation of 8; 4 = bytes requested, 8 = bytes allocated)
  size_t bytesRequested() const;
  
  // max number of bytes *requested*
  size_t maxBytesRequested() const;

  void walk(const std::function< void(Header *) >& f) const;

  // Return maximum size of object for a given size class.
  static size_t getSizeFromClass(int index);
  
  // Return the size class for a given size.
  static int getSizeClass(size_t sz);

private:

  recursive_mutex heapLock;
  static const auto Threshold = 16384;
  static const auto Base = 16;
  size_t allocated;
  size_t requested;
  size_t maxAllocated;
  size_t maxRequested;
  
  Header * allocatedObjects;
  Header * freedObjects[Threshold / Base + 32];
};

#include "statsalloc.cpp"

extern "C" {
  void reportStats();
  void walk(const std::function< void(Header *) >& f);
}

#endif
