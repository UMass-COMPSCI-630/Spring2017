#ifndef GCMALLOC_H
#define GCMALLOC_H

#include <cassert>
#include <functional>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <new>
#include <mutex>

#if defined(__APPLE__)
#include <mach-o/getsect.h>
#endif

extern "C" {
#if !defined(__APPLE__)
  extern int __data_start;
  extern int _end;
  
  extern char _etext;
  extern char _edata;
#endif
}

using namespace std;

extern void * GCMallocGlobal;

class Header {
public:
  bool isMarked() {
    return allocatedSize & 1;
  }
  void mark() {
    allocatedSize |= 1;
  }
  void clear() {
    allocatedSize &= ~1;
  }
  size_t getAllocatedSize() {
    return allocatedSize & ~1;
  }
  void setAllocatedSize(size_t sz) {
    allocatedSize = sz;
  }
  bool validateCookie() {
    return (cookie == ((uintptr_t) 0xdeadbeef ^ (uintptr_t) this));
  }
  void setCookie() {
    cookie = (uintptr_t) 0xdeadbeef ^ (uintptr_t) this;
  }
private:
  size_t cookie;  // magic number at the start of every object
  size_t allocatedSize; // how much space was allocated for it (mark bits encoded here)
public:
  // prev and next objects, whether allocated or freed.
  Header * prevObject;
  Header * nextObject;
};

template <class SourceHeap>
class GCMalloc : public SourceHeap {
public:
  GCMalloc();

  ~GCMalloc()
  {
  }
  
  enum { Alignment = 16 };

  // Allocate an object of at least the requested size.
  void * malloc(size_t sz);

  // Free an object.
  void free(void * ptr) {
    // in a garbage-collected context, free() is a NOP.
  }

  // Return the size of the given object.
  size_t getSize(void * p);

  // number of bytes currently allocated  
  size_t bytesAllocated();
  
  // Execute the given function on every allocated object.
  void walk(const std::function< void(Header *) >& f); 

  // Return maximum size of object for a given size class.
  static size_t getSizeFromClass(int index);
  
  // Return the size class for a given size.
  static int getSizeClass(size_t sz);

private:

  // Return the start and end of the stack.
  void getStack(void *& start, void *& end) {
    pthread_t self = pthread_self();
    auto addr = pthread_get_stackaddr_np(self);
    auto size = pthread_get_stacksize_np(self);
    // Assumes stack grows down.
    start = (void *) ((uintptr_t) addr - size);
    end = addr;
  }

  // Return the start and end of the globals.
  void getGlobals(void *& start, void *& end) {
    // currently a NOP.
#if defined(__APPLE__)
    // Note this is actually deprecated.
    start = (void *) get_etext();
    end   = (void *) get_end();
    if ((uintptr_t) &GCMallocGlobal < (uintptr_t) start) {
      start = &GCMallocGlobal;
    } else if ((uintptr_t) &GCMallocGlobal > (uintptr_t) end) {
      end = &GCMallocGlobal;
    }
#else
    start = (void *) &__data_start;
    end   = (void *) &_end;
#endif
  }

  // Scan through this region of memory looking for pointers to mark.
  void scan(void * start, void * end);
  
  // Indicate whether it is time to trigger a garbage collection
  // (call this inside your malloc).
  // Clearly this must happen when the heap is entirely full,
  // but for performance reasons, you should trigger GCs more frequently
  // (though not too frequently) using the fields below.
  bool triggerGC(size_t szRequested);

  // Track the amount of allocation since the last garbage collection.
  int bytesAllocatedSinceLastGC;

  // Track the amount of memory freed by the last garbage collection.
  int bytesReclaimedLastGC;
  
  // Perform a garbage collection pass.
  void gc();
  
  // Mark all reachable objects.
  void mark();

  // Mark one object as reachable and recursively mark everything reachable from it.
  void markReachable(void * ptr);

  // Reclaim all unreachable objects (add to free lists).
  void sweep();

  // Free one object.
  void privateFree(void *);

  // Returns true if the argument looks like a pointer that we allocated.
  // This should be as precise as possible without ignoring real allocated objects.
  // Just returning true is *not* an option :)
  bool isPointer(void * p);

  // The start of the heap (assume it is contiguous).
  void * startHeap;
  
  // The _current_ end of the heap (it should grow on demand).
  void * endHeap;

  // The lock that protects the heap.
  recursive_mutex heapLock;

  // Each object's size is rounded up to at least a multiple of Base.
  static const auto Base = 16;
  
  // We maintain exact size classes (multiples of Base) until this threshold.
  static const auto Threshold = 16384;

  // Number of objects allocated to date.
  size_t objectsAllocated;
  
  // The amount of memory currently allocated.
  size_t allocated;

  // The list of allocated objects.
  Header * allocatedObjects;

  // The lists of freed objects, organized by size classes.
  Header * freedObjects[Threshold / Base + 32];
};

#endif
