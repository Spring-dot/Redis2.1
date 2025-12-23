Step 3 changes overview

We will:

Add an LRUCache helper

Integrate it into DataStore

Evict on SET if capacity exceeded

Update LRU on GET

That’s it.

### How to test LRU (important)
Start server
./redis-lite

Interactive test
nc localhost 6379


Assume capacity = 3

SET a 1
SET b 2
SET c 3
SET d 4


Now:

GET a
(nil)      <-- evicted
GET b
2
GET c
3
GET d
4

Test “recently used survives”
SET a 1
SET b 2
SET c 3
GET a
SET d 4


Now:

GET b
(nil)   <-- b evicted, not a


Correct LRU behavior.