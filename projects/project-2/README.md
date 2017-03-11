Conservative garbage collection
===============================

 

(code templates at the [COMPSCI 630 Project 2
repo](https://github.com/UMass-COMPSCI-630/Spring2017/tree/master/projects/project-2
))

 

For this project (which is part two of a two-part project), you will adapt
the code from your memory allocator from part one and implement a conservative garbage
collector (with the same functionality as the Boehm-Demers-Weiser collector).
You will implement the missing functionality in the file `gcmalloc.cpp`.

Notice that the `Header` class has changed significantly. Rather than track
statistics, it now contains support for mark bits as well as a per-object "cookie"
that is used to check whether an object really was allocated by your allocator or now.
(This is all in the file `gcmalloc.hh`.)

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class Header {
public:
  size_t cookie;        // magic number at the start of every object
  size_t allocatedSize; // how much space was allocated for it (mark bits encoded here)
  Header * prevObject;  // previous object in linked list
  Header * nextObject;  // next object in linked list
};
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

As before, your allocator should be a segregated size class allocator.
However, when a user program calls `free`, your program should ignore
it.  At some point (which you will determine), you should trigger
garbage collections.  Your garbage collector will scan globals and the
stack for pointers, and mark all reachable objects. It will then sweep
the unreachable objects and add them to the appropriate free lists
(you should reuse your logic from your implementations of `free` and
`walk` from the first project).

As before, you will only be modifying a single file, `gcmalloc.cpp`. You need to
implement every method in that class.

You can trivially test your code by linking with the generated library; the
program `testme` is produced by the `Makefile`. You may need to set the
environment variable `LD_LIBRARY_PATH` as follows:

`export LD_LIBRARY_PATH=.`

To *really* test your code, replace the memory allocator in a real application
(if it crashes, you probably have a bug). This is straightforward to do on both
Mac OS X and Linux.

*On Mac:*

`export DYLD_INSERT_LIBRARIES=./libgcmalloc.dylib`

*On Linux:*

`export LD_PRELOAD=./libgcmalloc.so`

When you want to revert to the system allocator, do this:

*On Mac:*

`unset​ DYLD_INSERT_LIBRARIES`

*On Linux:*

`unset​ LD_PRELOAD`
