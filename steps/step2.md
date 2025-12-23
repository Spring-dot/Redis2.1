Step 2 = “make the server understand commands”

We changed only this:

1. Added an in-memory key–value store

std::unordered_map<string, string>

Shared by all clients

Owned by the single event loop thread

No locks

2. Replaced “echo everything” with line-based parsing

Client input buffered until \n

One command = one line

Prevents partial TCP reads from breaking commands

3. Implemented two commands
SET <key> <value>

Stores key → value

Responds with:

OK

GET <key>

If key exists → returns value

If not → returns:

(nil)

4. Still single-threaded, still epoll

No background threads

No TTL

No WAL

No eviction

This step is about correctness, not features.