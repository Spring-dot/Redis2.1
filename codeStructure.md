# Redis-Lite — Program Execution & Codebase Trace (Detailed)

This document traces **program execution**, **class composition**, and **file dependencies** in a compact, one-liner style, following the exact runtime flow.

---

## Program Entry

```
./redis-lite
→ OS loads executable
→ control enters main()
```

---

## main.cpp

### Includes

```cpp
#include "server.h"
```

### Execution

```
main.cpp
→ main() constructs Server(port = 6379)
→ main() calls server.run()
```

### Responsibility

* Owns process lifetime
* Entry point only
* Does not perform networking or I/O

---

## server.cpp / Server

### Includes

```cpp
#include "server.h"
#include "utils.h"
#include "event_loop.h"
```

### Class Members

```
Server
→ int port_
→ int listen_fd_
```

### Execution Flow

```
Server::run()
→ calls setup_socket()        // called exactly once
→ constructs EventLoop loop
→ loop.set_listen_fd(listen_fd_)
→ loop.run()                  // blocks forever
```

### Socket Setup

```
Server::setup_socket()
→ socket(AF_INET, SOCK_STREAM)
→ setsockopt(SO_REUSEADDR)
→ bind(port_)
→ listen(backlog)
→ utils::set_nonblocking(listen_fd_)
```

### Notes

* `listen_fd_` is a **single passive socket**
* Created once and reused for entire server lifetime
* Never accepts data directly

---

## utils.cpp

### Includes

```cpp
#include "utils.h"
#include <fcntl.h>
#include <unistd.h>
```

### Functionality

```
utils::set_nonblocking(fd)
→ fcntl(F_GETFL)
→ fcntl(F_SETFL | O_NONBLOCK)
```

### Purpose

* Ensures all sockets are non-blocking
* Required for correct epoll-based server behavior

---

## event_loop.cpp / EventLoop

### Includes

```cpp
#include "event_loop.h"
#include "client.h"
#include "datastore.h"
#include "utils.h"
#include <sys/epoll.h>
```

### Class Members

```
EventLoop
→ int epfd_
→ int listen_fd_
→ unordered_map<int, Client*> clients_
→ DataStore store_
```

### Initialization

```
EventLoop::set_listen_fd(fd)
→ stores listen_fd_
```

### Main Loop

```
EventLoop::run()
→ epoll_create1()
→ add_epoll(listen_fd_, EPOLLIN)

while (true):
    → epoll_wait()
    → for each event:
```

### Handling New Connections

```
if fd == listen_fd_:
→ accept() in loop
→ utils::set_nonblocking(client_fd)
→ new Client(client_fd)
→ clients_[client_fd] = Client*
→ add_epoll(client_fd, EPOLLIN)
```

### Handling Client I/O

```
else (client_fd):
    if EPOLLIN:
        → client->handle_read(store_)
        → if false: cleanup client
        → if client->has_pending_write():
              mod_epoll(fd, EPOLLIN | EPOLLOUT)

    if EPOLLOUT:
        → client->handle_write()
        → if outbuf empty:
              mod_epoll(fd, EPOLLIN)
```

### Epoll Helpers

```
add_epoll(fd, flags)
mod_epoll(fd, flags)
del_epoll(fd)
```

---

## client.cpp / Client

### Includes

```cpp
#include "client.h"
#include "datastore.h"
#include <unistd.h>
#include <sstream>
```

### Class Members

```
Client
→ int fd_
→ string inbuf_
→ string outbuf_
→ protocol parsing state
```

### Read Path

```
Client::handle_read(store)
→ read(fd_) in non-blocking loop
→ append bytes to inbuf_
→ parse commands incrementally
→ execute GET / SET / EXPIRE via DataStore
→ append responses to outbuf_
→ return false on disconnect/error
```

### Write Path

```
Client::handle_write()
→ write(fd_, outbuf_)
→ erase written bytes
```

### Cleanup

```
Client::~Client()
→ close(fd_)
```

---

## datastore.cpp / DataStore

### Includes

```cpp
#include "datastore.h"
#include "lru.h"
#include "ttl.h"
#include "wal.h"
```

### Class Members

```
DataStore
→ unordered_map<string, string> store_
→ LRUCache lru_
→ TTL ttl_
→ WAL* wal_   // optional
```

### SET

```
DataStore::set(key, value)
→ store_[key] = value
→ lru_.touch / insert
→ ttl_.erase(key)
→ wal_->log(SET ...) if attached
→ evict LRU if capacity exceeded
```

### GET

```
DataStore::get(key)
→ if ttl_.expired(key):
      del(key)
      return false
→ return value
→ lru_.touch(key)
```

### EXPIRE

```
DataStore::expire(key, seconds)
→ ttl_.set(key, seconds)
→ wal_->log(EXPIRE ...)
```

### DELETE (internal)

```
DataStore::del(key)
→ erase from store_
→ erase from lru_
→ erase from ttl_
```

---

## lru.cpp / LRUCache

### Includes

```cpp
#include "lru.h"
```

### Structure

```
LRUCache
→ unordered_map<key, list_iterator>
→ list<key> recency_list
```

### Operations

```
touch(key)  → move to front
insert(key) → push front
evict()     → pop back (least recent)
```

---

## ttl.cpp / TTL

### Includes

```cpp
#include "ttl.h"
#include <chrono>
#include <random>
```

### Structure

```
TTL
→ unordered_map<key, expiry_time>
```

### Operations

```
set(key, seconds)
→ expiry_[key] = now + seconds

expired(key)
→ compare steady_clock::now()

sample_cleanup()
→ randomly remove expired keys
```

---

## wal.cpp / WAL (if enabled)

### Includes

```cpp
#include "wal.h"
#include "datastore.h"
#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>
```

### Structure

```
WAL
→ background writer thread
→ mutex + condition_variable
→ queue<string> log queue
```

### Logging Path

```
DataStore → wal_->log(record)
→ enqueue record
→ notify writer thread
```

### Writer Thread

```
writer_loop()
→ dequeue records
→ append to wal.log
→ flush periodically
```

### Recovery

```
WAL::replay(store)
→ read wal.log line-by-line
→ reapply SET / EXPIRE to DataStore
```

---

## End-to-End Mental Model

```
main
→ Server
→ EventLoop
→ Client
→ DataStore
→ (LRU + TTL + WAL)
```

---

## Lifetime Summary

```
Server        : process lifetime
listen_fd    : created once, never changes
EventLoop    : single instance
Client        : one per TCP connection
DataStore    : single shared instance
LRU / TTL    : owned by DataStore
WAL thread   : background only
```

---


