#include "utils.h"

void print_error(const char *msg) {
    perror(msg);
}

int send_all(int socket, const void *buffer, size_t length) {
    const char *ptr = (const char *)buffer;
    while (length > 0) {
        ssize_t i = send(socket, ptr, length, 0);
        if (i < 1) return -1;
        ptr += i;
        length -= i;
    }
    return 0;
}

int recv_all(int socket, void *buffer, size_t length) {
    char *ptr = (char *)buffer;
    while (length > 0) {
        ssize_t i = recv(socket, ptr, length, 0);
        if (i < 1) return -1;
        ptr += i;
        length -= i;
    }
    return 0;
}
