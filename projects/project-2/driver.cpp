#include "gcmalloc.hh"
#include "mmapheap.h"

const auto HeapSize = 1024UL * 1024 * 1024;
class HeapType : public GCMalloc<MmapHeap<HeapSize>> {};

static HeapType& getHeap() {
  static char theHeapBuf[sizeof(HeapType)];
  static HeapType * h = new (theHeapBuf) HeapType;
  return *h;
}

extern "C" {

  void walk(std::function< void(Header *) > f)
  {
    getHeap().walk(f);
  }
  
  void * xxmalloc(size_t sz)
  {
    return getHeap().malloc(sz);
  }
  
  void xxfree(void * ptr) {
    getHeap().free(ptr);
  }
  
  size_t xxmalloc_usable_size(void * ptr) {
    return getHeap().getSize(ptr);
  }
  
  void xxmalloc_lock() {
  }
  
  void xxmalloc_unlock() {
  }
}
