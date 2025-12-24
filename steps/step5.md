Step: TTL (Key Expiry)
What TTL will do

New command added
EXPIRE <key> <seconds>\n

Sets a TTL on an existing key

Returns:

1 → TTL set

0 → key does not exist

On GET / SET: expired keys are removed lazily

Optional active cleanup: sample a few keys periodically

No threads. No locks. Single-threaded and deterministic.

How to test Step 5 (IMPORTANT)
1️⃣ Basic TTL test
nc localhost 6379


Then type:

SET a hello
OK
EXPIRE a 2
1
GET a
hello


Wait ~2 seconds, then:

GET a
(nil)


✅ TTL works.

2️⃣ EXPIRE on missing key
EXPIRE missing 10
0


Correct behavior.

3️⃣ TTL + overwrite
SET x 1
EXPIRE x 5
SET x 2
GET x
2


TTL is cleared implicitly when overwritten (because old TTL entry is removed).

4️⃣ TTL + LRU interaction

Assume LRU capacity = 3.

SET a 1
SET b 2
SET c 3
EXPIRE a 1


Wait ~1s, then:

SET d 4


Now:

GET a
(nil)
GET b
2
GET c
3
GET d
4


Expired keys do not block LRU eviction.

5️⃣ Multiple keys expiring
SET k1 v1
SET k2 v2
SET k3 v3
EXPIRE k1 1
EXPIRE k2 2
EXPIRE k3 3


Then check progressively:

GET k1   -> (nil)
GET k2   -> (nil) after 2s
GET k3   -> (nil) after 3s