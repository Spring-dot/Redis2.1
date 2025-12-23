✅ Step 1: Implement TCP server + epoll + client read/write
Goal

Server listens on a port

Accepts new TCP clients

Reads incoming data

Echoes it back

Non-blocking sockets

Single-threaded event loop

Expected behavior

You run:

./redis-lite


Then from another terminal:

nc localhost 6379
hello
hello


If that works → foundation is solid.

Step 1 Detailed Breakdown

You need to implement:

1️⃣ Setup listening socket

Inside Server::setup_socket():

socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)

bind()

listen()

utils::set_nonblocking()

2️⃣ epoll loop recognizes:

new connections on listen_fd

readable event on a client fd

writable event on a client fd

3️⃣ Client object:

maintain fd

input buffer

output buffer

4️⃣ Protocol (temporary):

no parsing yet

simply echo raw bytes

### TESTING/UNDERSTANDING

2️⃣ Execution flow (trace this once)

Do this with a pen or debugger:

main() → Server::run()

setup_socket():

socket

bind

listen

non-blocking

EventLoop::run()

epoll_wait()

Event arrives:

listen_fd → accept()

client_fd → read/write

Client disconnect → cleanup

If you can trace this without looking at code → you understand it.

................

commands:
make clean
make 
./redis-lite
In another terminal:
nc localhost 6379
hello