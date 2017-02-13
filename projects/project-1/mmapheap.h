#ifndef MMAPHEAP_H
#define MMAPHEAP_H

#include <sys/mman.h>

template <size_t Size>
class MmapHeap {
public:
  MmapHeap()
    : heapRemaining (Size)
  {
    origHeapStart = (char *) mmap((void *) 0, Size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (origHeapStart == (char *) -1) {
      perror("Map failed");
      origHeapStart = nullptr;
      heapRemaining = 0;
    }
    heapStart = origHeapStart;
  }
  ~MmapHeap() {
    if (origHeapStart != nullptr) {
      munmap(origHeapStart, Size);
    }
  }
  void * malloc(size_t sz) {
    if (heapRemaining < sz) {
      return nullptr;
    }
    char * p = heapStart;
    heapStart += sz;
    heapRemaining -= sz;
    return (void *) p;
  }
private:
  void free(void *);
  size_t getSize(void *);
  char * origHeapStart;
  char * heapStart;
  size_t heapRemaining;
};


#endif
