#ifndef GCMALLOC_H
#define GCMALLOC_H

#include <cassert>
#include <functional>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <new>
#include <mutex>
#include <cstring>

#include "tprintf.hh"
#include "os_specific.hh"

using namespace std;

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

  // Needed for malloc replacement.
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
  static int constexpr getSizeClass(size_t sz);

private:

  // Scan through this region of memory looking for pointers to mark (and mark them).
  void scan(void * start, void * end);
  
  // Indicate whether it is time to trigger a garbage collection
  // (call this inside your malloc).
  // Clearly this must happen when the heap is entirely full,
  // but for performance reasons, you should trigger GCs more frequently
  // (though not too frequently) using the fields below.
  bool triggerGC(size_t szRequested);

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

  // Access to OS specific routines to walk stack and globals.
  OSSpecific sp;
  
  // Track the amount of allocation since the last garbage collection.
  long bytesAllocatedSinceLastGC;

  // Track the amount of memory freed by the last garbage collection.
  long bytesReclaimedLastGC;
  
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

  // Is everything ready? If not, malloc should just request from the
  // source heap and return that memory.
  bool initialized;

  // Are we currently in a GC? (used to avoid reentrancy issues)
  bool inGC;

  // After allocating this many bytes, we can trigger a GC (optional).
  long nextGC;
  
  /// Quickly calculate the CEILING of the log (base 2) of the argument.
#if defined(_WIN32)
  static inline int log2 (size_t sz) 
  {
    int retval;
    sz = (sz << 1) - 1;
    __asm {
      bsr eax, sz
	mov retval, eax
	}
    return retval;
  }
#elif defined(__GNUC__) && defined(__i386__)
  static inline int log2 (size_t sz) 
  {
    sz = (sz << 1) - 1;
    asm ("bsrl %0, %0" : "=r" (sz) : "0" (sz));
    return (int) sz;
  }
#elif defined(__GNUC__) && defined(__x86_64__)
  static inline int log2 (size_t sz) 
  {
    sz = (sz << 1) - 1;
    asm ("bsrq %0, %0" : "=r" (sz) : "0" (sz));
    return (int) sz;
  }
#elif defined(__GNUC__)
  // Just use the intrinsic.
  static inline constexpr int log2 (size_t sz) 
  {
    sz = (sz << 1) - 1;
    return (sizeof(unsigned long) * 8) - __builtin_clzl(sz) - 1;
  }
#else
  static inline int log2 (size_t v) {
    int log = 0;
    unsigned int value = 1;
    while (value < v) {
      value <<= 1;
      log++;
    }
    return log;
  }
#endif
  
};

#endif
