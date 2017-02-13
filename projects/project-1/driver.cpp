#include "statsalloc.h"
#include "mmapheap.h"

const auto HeapSize = 1024UL * 1024 * 1024;
class HeapType : public StatsAlloc<MmapHeap<HeapSize>> {};

static HeapType& getHeap() {
  static char theHeapBuf[sizeof(HeapType)];
  static HeapType * h = new (theHeapBuf) HeapType;
  return *h;
}

extern "C" {

  void walk(const std::function< void(Header *) >& f)
  {
    getHeap().walk(f);
  }
  
  void reportStats()
  {
    char buf[255];
    sprintf(buf, "Max requested = %zu\n", getHeap().maxBytesRequested());
    printf(buf);
    sprintf(buf, "Max allocated = %zu\n", getHeap().maxBytesAllocated());
    printf(buf);
    sprintf(buf, "Bytes requested = %zu\n", getHeap().bytesRequested());
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

#if 0
int
main()
{
#if 1
  HeapType heapo;
#if 0
  for (int i = 0; i < 65536 * 4; i++) {
    auto cl = heapo.getSizeClass(i);
    cout << i << ": " << cl << ", " << heapo.getSizeFromClass(cl) << endl;
  }
#endif
  for (int i = 0; i < 10; i++) {
    void * ptr = heapo.malloc(1);
  }
  for (int i = 0; i < 10; i++) {
    void * ptr = heapo.malloc(1);
    cout << ptr << endl;
    heapo.free(ptr);
  }
  void * ptr = heapo.malloc(4);
  size_t req = 0;
  heapo.walk([&](HeapType::Header * h){ req += h->requestedSize; });
  cout << req << endl;
  cout << heapo.bytesRequested() << endl;
  size_t alloc = 0;
  heapo.walk([&](HeapType::Header * h){ alloc += h->allocatedSize; });
  cout << alloc << endl;
  cout << heapo.bytesAllocated() << endl;
  cout << heapo.maxBytesAllocated() << endl;
#endif
  return 0;
}
#endif
