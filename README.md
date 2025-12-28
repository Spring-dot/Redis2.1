# Redis2.1 (C++ High-Performance In-Memory Key-Value Store)
- creating redis from scratch using C/C++

Built a high-performance in-memory key-value store in C++17 inspired by Redis, supporting concurrent clients, sharded locking, LRU eviction, TTL expiration, and persistent recovery via WAL, achieving low-latency reads under multi-threaded load.

Designed and implemented a Redis-inspired in-memory datastore in C++ with sharded concurrency control, LRU eviction, TTL expiration, and write-ahead logging, achieving ~XXX QPS with XX concurrent clients.

Implemented a Redis-inspired in-memory key-value store in C++ using a single-threaded epoll-based event loop, lock-free command execution, LRU eviction, TTL expiration, and asynchronous write-ahead logging for crash recovery.
 
---

# Redis2.1 — A Redis-Inspired In-Memory Key-Value Store (C++)

## Overview

**Redis-Lite** is a Redis-inspired, in-memory key-value datastore implemented in **modern C++ (C++17)**.

The project focuses on **core systems concepts** rather than feature parity with Redis:

* single-threaded, event-driven server (`epoll`)
* deterministic command execution (no locks on the hot path)
* exact **LRU eviction**
* **TTL / key expiry** (lazy + active)
* length-prefixed, binary-safe protocol
* clean separation of concerns
* extensible design (persistence via WAL planned)

This is **not a Redis clone**.
---

## Key Design Principles

### 1. Single-Threaded Core

All command execution and data mutation happen in **one thread**.

Why:

* no locks on the hot path
* deterministic behavior
* simpler correctness reasoning
* matches Redis’s core philosophy

Blocking operations (e.g. persistence) are planned to be offloaded to background threads.

---

### 2. Event-Driven Networking

The server uses:

* non-blocking TCP sockets
* `epoll` for I/O multiplexing
* per-client input/output buffers

This allows:

* many concurrent clients
* no thread-per-connection overhead
* correct handling of partial reads/writes

---

### 3. Explicit Tradeoffs

This project intentionally:

* prioritizes clarity over micro-optimizations
* implements *exact* LRU (not approximate)
* uses lazy expiry instead of timers
* avoids protocol guesswork

Every simplification is **conscious and documented**.

---

## Architecture Overview

```
┌─────────────────────────────┐
│           Clients           │
│     (nc / Python / etc.)    │
└──────────────┬──────────────┘
               │ TCP
┌──────────────▼──────────────┐
│        epoll Event Loop     │
│   (single-threaded core)    │
└──────────────┬──────────────┘
               │
      ┌────────▼────────┐
      │   Client State  │
      │  (buffers + FSM)│
      └────────┬────────┘
               │
┌──────────────▼──────────────┐
│          DataStore          │
│  ┌─────────┐  ┌─────────┐  │
│  │  LRU    │  │  TTL    │  │
│  └─────────┘  └─────────┘  │
└─────────────────────────────┘
```

---

## Directory Structure

```
redis-lite/
├── include/
│   ├── server.h        # Server bootstrap
│   ├── event_loop.h    # epoll loop
│   ├── client.h        # Per-client state & parsing
│   ├── datastore.h    # Key-value store
│   ├── lru.h           # LRU eviction
│   ├── ttl.h           # TTL / expiry
│   └── utils.h         # Helpers
├── src/
│   ├── server.cpp
│   ├── event_loop.cpp
│   ├── client.cpp
│   ├── datastore.cpp
│   ├── lru.cpp
│   ├── ttl.cpp
│   └── utils.cpp
├── Makefile
└── README.md
```

---

## Supported Commands

### SET

```
SET <key> <length>
<raw value bytes>
```

* Stores a value under `key`
* Value is **binary-safe**
* `<length>` specifies **exact number of bytes**
* Overwriting a key clears its previous TTL

Response:

```
OK
```

---

### GET

```
GET <key>
```

Responses:

* If key exists and not expired:

  ```
  <length>
  <raw value bytes>
  ```
* If key does not exist or expired:

  ```
  (nil)
  ```

---

### EXPIRE

```
EXPIRE <key> <seconds>
```

* Sets a TTL (time-to-live) on an existing key
* TTL is in **seconds**

Responses:

* `1` → TTL set successfully
* `0` → key does not exist

---

## TTL (Key Expiry) Semantics

TTL implementation matches Redis philosophy (simplified).

### Lazy Expiration

* On every `GET`
* If key is expired:

  * it is deleted immediately
  * behaves as `(nil)`

### Active Expiration (Sampling)

* Periodically samples a small number of TTL keys
* Deletes expired ones
* Prevents memory from filling with dead keys

### Notes

* Expiry is **best-effort**, not exact-time
* No background timers
* No per-key wakeups

---

## LRU Eviction

Redis-Lite enforces a **fixed capacity** (number of keys).

### Policy

* Exact **Least Recently Used (LRU)**
* Access via `GET` or `SET` updates recency
* On overflow:

  * least recently used key is evicted

### Data Structures

* `unordered_map<key, value>`
* doubly-linked list for recency tracking

All operations are **O(1)**.

---

## Protocol Design Notes

### Why Length-Prefixed?

* Supports binary values
* Avoids ambiguity
* Correct for stream-based TCP

### Terminal UX Caveat

When using `nc`:

* pressing Enter sends an extra `\n`
* this byte is **not part of the value unless included in length**

For binary or multiline values:

* use a programmatic client (Python, etc.)

This mirrors Redis behavior:

* Redis server is binary-strict
* Redis CLI hides protocol complexity

---

## Build Instructions

```
make clean
make
```

---

## Running the Server

```
./redis-lite
```

The server listens on port **6379**.

---

## Testing Examples

### Basic SET / GET

```
nc localhost 6379
SET a 5
hello
OK
GET a
5
hello
```

---

### TTL Test

```
SET k 3
abc
EXPIRE k 2
(wait 2 seconds)
GET k
(nil)
```

---

### Programmatic Binary Test (Recommended)

```python
import socket

s = socket.socket()
s.connect(("127.0.0.1", 6379))

val = b"b\nc\nc\nd"
s.sendall(b"SET a " + str(len(val)).encode() + b"\n" + val)
print(s.recv(1024))

s.sendall(b"GET a\n")
print(s.recv(1024))
```

---

## What Is NOT Implemented (Yet)

* Persistence (WAL) — **next step**
* Replication
* Clustering
* RESP protocol
* Authentication

---

## Planned Next Feature: WAL (Persistence)

Write-Ahead Logging will add:

* crash recovery
* background I/O thread
* mutex + condition variable
* durability tradeoffs

This will showcase:

* multithreading
* producer–consumer design
* safe concurrency

---

## Interview-Ready Summary

> Redis-Lite is a Redis-inspired, single-threaded, event-driven in-memory datastore implemented in C++.
> It supports binary-safe values, exact LRU eviction, TTL expiry, and is designed for correctness and clarity rather than feature parity.

---

## License

Educational / personal project.

---


