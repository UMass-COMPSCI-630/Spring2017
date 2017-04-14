## tl;dr
**Supported**:

* Text protocol
* TCP connections
* Storage commands:  All of them (`set`, `add`, `replace`, `append`, `prepend`, `cas`)
* Retrieval commands: All of them (`get`, `gets`, `delete`, `incr/decr`)
* Stats commands: Some of them (`stats`, maybe `stats items`)
* Misc commands: All of them (`flush_all`, `version`, `quit`)

**Unsupported**:

* Binary protocol
* UDP
* SASL connections
* Stats commands: `stats <foobar>`

## Overview

Memcached has a set of simple commands and two protocols for invoking them: A binary protocol, and a text protocol.

Text protocol: https://github.com/memcached/memcached/blob/master/doc/protocol.txt
Binary protocol: https://code.google.com/p/memcached/wiki/BinaryProtocolRevamped

Furthermore, Memcached supports TCP and UDP, with TCP being the default for reliability.

You only need to implement the more-commonly-used text protocol over TCP only. That said, with proper abstractions, adding UDP later would be trivial, as would adding the binary protocol (since it maps directly onto the text commands). Adding those is optional.

Memcached also supports an encrypted protocol, but you do not need to support it: https://code.google.com/p/memcached/wiki/SASLAuthProtocol

## Storage Commands
Descriptions lifted from this document: https://code.google.com/p/memcached/wiki/NewCommands

### `set`
> Most common command. Store this data, possibly overwriting any existing data. New items are at the top of the LRU.

This is essential for memcached.

### `add`
> Store this data, only if it does not already exist. New items are at the top of the LRU. If an item already exists and an add fails, it promotes the item to the front of the LRU anyway.

Similarly essential.

### `replace`
> Store this data, but only if the data already exists. Almost never used, and exists for protocol completeness (set, add, replace, etc)

Would be simple to add, even if "almost never used" (you *know* that *someone* has used it).

### `append`
> Add this data after the last byte in an existing item. This does not allow you to extend past the item limit. Useful for managing lists.

Essential.

### `prepend`
> Same as append, but adding new data before existing data.

Essential.

### `cas`
> Check And Set (or Compare And Swap). An operation that stores data, but only if no one else has updated the data since you read it last. Useful for resolving race conditions on updating cache data.

Essential.

## Retrieval Commands

### `get`
> Command for retrieving data. Takes one or more keys and returns all found items.

Essential.

### `gets`
> An alternative get command for using with CAS. Returns a CAS identifier (a unique 64bit number) with the item. Return this value with the cas command. If the item's CAS value has changed since you gets'ed it, it will not be stored.

Essential (easy since you are already implementing CAS).

### `delete`
> Removes an item from the cache, if it exists.

Essential.

### `incr/decr`
> Increment and Decrement. If an item stored is the string representation of a 64bit integer, you may run incr or decr commands to modify that number. You may only incr by positive values, or decr by positive values. They does not accept negative values. If a value does not already exist, incr/decr will fail.

Essential.

## Stats Commands

### `stats`
> ye olde basic stats command.

It returns the following stats:
```
|-----------------------+---------+-------------------------------------------|
| Name                  | Type    | Meaning                                   |
|-----------------------+---------+-------------------------------------------|
| pid                   | 32u     | Process id of this server process         |
| uptime                | 32u     | Number of secs since the server started   |
| time                  | 32u     | current UNIX time according to the server |
| version               | string  | Version string of this server             |
| pointer_size          | 32      | Default size of pointers on the host OS   |
|                       |         | (generally 32 or 64)                      |
| rusage_user           | 32u.32u | Accumulated user time for this process    |
|                       |         | (seconds:microseconds)                    |
| rusage_system         | 32u.32u | Accumulated system time for this process  |
|                       |         | (seconds:microseconds)                    |
| curr_items            | 32u     | Current number of items stored            |
| total_items           | 32u     | Total number of items stored since        |
|                       |         | the server started                        |
| bytes                 | 64u     | Current number of bytes used              |
|                       |         | to store items                            |
| curr_connections      | 32u     | Number of open connections                |
| total_connections     | 32u     | Total number of connections opened since  |
|                       |         | the server started running                |
| connection_structures | 32u     | Number of connection structures allocated |
|                       |         | by the server                             |
| reserved_fds          | 32u     | Number of misc fds used internally        |
| cmd_get               | 64u     | Cumulative number of retrieval reqs       |
| cmd_set               | 64u     | Cumulative number of storage reqs         |
| cmd_flush             | 64u     | Cumulative number of flush reqs           |
| cmd_touch             | 64u     | Cumulative number of touch reqs           |
| get_hits              | 64u     | Number of keys that have been requested   |
|                       |         | and found present                         |
| get_misses            | 64u     | Number of items that have been requested  |
|                       |         | and not found                             |
| delete_misses         | 64u     | Number of deletions reqs for missing keys |
| delete_hits           | 64u     | Number of deletion reqs resulting in      |
|                       |         | an item being removed.                    |
| incr_misses           | 64u     | Number of incr reqs against missing keys. |
| incr_hits             | 64u     | Number of successful incr reqs.           |
| decr_misses           | 64u     | Number of decr reqs against missing keys. |
| decr_hits             | 64u     | Number of successful decr reqs.           |
| cas_misses            | 64u     | Number of CAS reqs against missing keys.  |
| cas_hits              | 64u     | Number of successful CAS reqs.            |
| cas_badval            | 64u     | Number of CAS reqs for which a key was    |
|                       |         | found, but the CAS value did not match.   |
| touch_hits            | 64u     | Numer of keys that have been touched with |
|                       |         | a new expiration time                     |
| touch_misses          | 64u     | Numer of items that have been touched and |
|                       |         | not found                                 |
| auth_cmds             | 64u     | Number of authentication commands         |
|                       |         | handled, success or failure.              |
| auth_errors           | 64u     | Number of failed authentications.         |
| evictions             | 64u     | Number of valid items removed from cache  |
|                       |         | to free memory for new items              |
| reclaimed             | 64u     | Number of times an entry was stored using |
|                       |         | memory from an expired entry              |
| bytes_read            | 64u     | Total number of bytes read by this server |
|                       |         | from network                              |
| bytes_written         | 64u     | Total number of bytes sent by this server |
|                       |         | to network                                |
| limit_maxbytes        | 32u     | Number of bytes this server is allowed to |
|                       |         | use for storage.                          |
| threads               | 32u     | Number of worker threads requested.       |
|                       |         | (see doc/threads.txt)                     |
| conn_yields           | 64u     | Number of times any connection yielded to |
|                       |         | another due to hitting the -R limit.      |
| hash_power_level      | 32u     | Current size multiplier for hash table    |
| hash_bytes            | 64u     | Bytes currently used by hash tables       |
| hash_is_expanding     | bool    | Indicates if the hash table is being      |
|                       |         | grown to a new size                       |
| expired_unfetched     | 64u     | Items pulled from LRU that were never     |
|                       |         | touched by get/incr/append/etc before     |
|                       |         | expiring                                  |
| evicted_unfetched     | 64u     | Items evicted from LRU that were never    |
|                       |         | touched by get/incr/append/etc.           |
| slab_reassign_running | bool    | If a slab page is being moved             |
| slabs_moved           | 64u     | Total slab pages moved                    |
|-----------------------+---------+-------------------------------------------|
```

You can return fake values for some of these (e.g. `hash_is_expanding`? Always false).

### `stats items`, `stats slabs`, and `stats <foobar>`
From https://github.com/memcached/memcached/blob/master/doc/protocol.txt:
> stats \<args\>\r\n
> 
>Depending on \<args\>, various internal data is sent by the server. The kinds of arguments and the data sent are not documented in this version of the protocol, and are subject to change for the convenience of memcache developers.

Does not need to be implemented.

That said, there's documentation for what memcached sends now, and some of them may be reasonable to implement. See ``protocol.txt`` for details.


## Misc Commands

### `flush_all`
> Invalidate all existing cache items. Optionally takes a parameter, which means to invalidate all items after N seconds have passed. This command does not pause the server, as it returns immediately. It does not free up or flush memory at all, it just causes all items to expire.

### `version`

### `quit`
> Upon receiving this command, the server closes the connection. However, the client may also simply close the connection when it no longer needs it, without issuing this command.
