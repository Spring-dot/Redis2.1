#include "utils.h"
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>

void utils::set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl(F_GETFL) failed");
        exit(EXIT_FAILURE);
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl(F_SETFL) failed");
        exit(EXIT_FAILURE);
    }
}
