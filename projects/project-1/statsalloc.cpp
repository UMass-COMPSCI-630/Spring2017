template <class SourceHeap>
void * StatsAlloc<SourceHeap>::malloc(size_t sz) {
  return nullptr; // FIX ME
}
  
template <class SourceHeap>
void StatsAlloc<SourceHeap>::free(void * ptr) {
  // FIX ME
}

template <class SourceHeap>
size_t StatsAlloc<SourceHeap>::getSize(void * p) {
  return 0; // FIX ME
}

// number of bytes currently allocated  
template <class SourceHeap>
size_t StatsAlloc<SourceHeap>::bytesAllocated() {
  return 0; // FIX ME
}

// max number of bytes allocated  
template <class SourceHeap>
size_t StatsAlloc<SourceHeap>::maxBytesAllocated() {
  return 0; // FIX ME
}

// number of bytes *requested* (e.g., malloc(4) might result in an allocation of 8; 4 = bytes requested, 8 = bytes allocated)
template <class SourceHeap>
size_t StatsAlloc<SourceHeap>::bytesRequested() {
  return 0; // FIX ME
}
  
// max number of bytes *requested*
template <class SourceHeap>
size_t StatsAlloc<SourceHeap>::maxBytesRequested() {
  return 0; // FIX ME
}

template <class SourceHeap>
void StatsAlloc<SourceHeap>::walk(const std::function< void(Header *) >& f) {
  // FIX ME
}

template <class SourceHeap>
size_t StatsAlloc<SourceHeap>::getSizeFromClass(int index) {
  return 0; // FIX ME
}


template <class SourceHeap>
int StatsAlloc<SourceHeap>::getSizeClass(size_t sz) {
  return 0; // FIX ME
}
