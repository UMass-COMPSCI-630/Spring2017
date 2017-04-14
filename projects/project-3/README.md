Project 3 - Memo (a memcached clone)

For this project, you will write a multithreaded server that
implements the memcached protocol in C++11.  Due date is May 8.

What is memcached? See http://memcached.org/ and
http://en.wikipedia.org/wiki/Memcached.

Basically, it is a glorified in-memory hash table. You can store
values with a given key, and then retrieve the value later using that
key. However, the value may have been evicted from memory.

Your group should all work together to write all of the code in your
system; however, you are free (and welcome) to discuss issues like the
software architecture, test cases, etc. on Piazza. If you are unsure
as to what would constitute plagiarism, please contact me.

Your submission needs to include a Makefile that builds the whole thing,
and a "test" target so that we can easily run your tests.

In addition to the usual grading criteria, there will be bonus points
for speed and efficiency. The faster and more memory-efficient it is,
the higher your bonus will be.

Make sure to make the cache replacement part pluggable. You need to implement
_three_ different replacement policies:

* LRU (least-randomly used)
* Random replacement
* Landlord (which takes recency, size, and cost into account; see http://www.cs.ucr.edu/~neal/Young02Online.pdf, page 73).

You should expose these choices as command-line arguments, as in: `-lru`, `-random` and `-landlord`.

Note that for the last case, you need to _infer_ the time cost of
fetching an item by tracking misses and matching them with insertions
into the cache (assuming that this is a good proxy for the cost).

You should write your code to work as a daemon that just maintains a
large shared memory mapping.  Start the daemon with this line to
create a 2GB cache. You should not use more memory than this to hold
your cache; you will need to keep track of allocated memory to ensure
you don't exceed this size.

   % memo 2048M

Use the protocol described in `protocol.md`. You only need to implement the text
protocol and commands described as essential in `requirements.md`.

