Statistics-gathering memory allocator
=====================================

 

(code templates at the [COMPSCI 630 Project 1
repo](https://github.com/UMass-COMPSCI-630/Spring2017/tree/master/projects/project-1
))

 

For this project (which is part one of a two-part project), you will write a
replacement memory allocator in C++ whose goal is to collect memory usage
statistics *and* lay the groundwork for the second part, a conservative garbage
collector (with the same functionality as the Boehm-Demers-Weiser collector).
Note that the scaffolding code is provided for you: you will implement the
missing functionality in the file `statsalloc.cpp`.

In addition to the usual memory allocation functionality (malloc and free), your
allocator will provide functions that allow querying about memory allocator
usage:

-   `size_t bytesAllocated()` -- number of bytes currently allocated

-   `size_t maxBytesAllocated()` -- max number of bytes allocated

-   `size_t bytesRequested()` -- number of bytes *requested* (e.g., malloc(4)
    might result in an allocation of 8; 4 = bytes requested, 8 = bytes
    allocated)

-   `size_t maxBytesRequested()` -- max number of bytes \*requested\*

To support this functionality, your allocator will use per-object headers (class
`Header`) that should immediately precede each object. The headers will store
the requested object size and the allocated object size (always at least as
large as requested), as well as other info. (This is all in the file
`statsalloc.h`.)

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class Header {
public:
  size_t requestedSize;  // how big was this object request
  size_t allocatedSize; // how much space was allocated for it
  Header * prevObject; // previous object in linked list
  Header * nextObject; // next object in linked list
};
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

You should implement the allocator as a *segregated size class* allocator. There
will be an array of free lists (of freed objects), chained together through the
object headers, and each array element should hold objects of a single (rounded)
size. Your size classes should be exact multiples of 16 for every size up to
16384, and then powers of two from 16,384 to 512 MB. Larger object requests
should return NULL.

Your allocator will also implement a `walk` function that will allow enumerating
objects via a map-like interface. Note that this takes C++ lambdas as arguments;
you *must* compile your program using the C++ 14 standard (this is already in
the `Makefile` for `clang++`).

All allocated memory should come from a single large chunk of memory allocated
with `mmap` (this logic is provided for you).

Finally, your allocator must be *thread-saf*e: it does not need to scale, but
key data structures must be protected by locks, and care needs to be taken so
that everything works in the presence of multiple threads.

You will only be modifying a single file, `statsalloc.cpp`. You need to
implement every method in that class.

You can trivially test your code by linking with the generated library; the
program `testme` is produced by the `Makefile`. You may need to set the
environment variable `LD_LIBRARY_PATH` as follows:

`export LD_LIBRARY_PATH=.`

To *really* test your code, replace the memory allocator in a real application
(if it crashes, you probably have a bug). This is straightforward to do on both
Mac OS X and Linux.

*On Mac:*

`export DYLD_INSERT_LIBRARIES=./libstatsalloc.dylib`

*On Linux:*

`export LD_PRELOAD=./libstatsalloc.so`

When you want to revert to the system allocator, do this:

*On Mac:*

`unset​ DYLD_INSERT_LIBRARIES`

*On Linux:*

`unset​ LD_PRELOAD`
