#include "../../include/utils.h"

void print_error(const char *msg) {
    perror(msg);
}

/*
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
*/
int send_all(int socket, const void *buffer, size_t length) {
    int bytes_sent = 0, ret;
    const char *ptr = (const char *)buffer;
    while (bytes_sent < length)
    {
        ret = send(socket, ptr + bytes_sent, length - bytes_sent, 0);
        if (ret == -1 && errno == EINTR)
            continue;
        bytes_sent += ret;
    }
    return bytes_sent;
}

int send_string(int client_socket, char *str) {
    int length = strlen(str);
    send_all(client_socket, str, length);
}

/*
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
*/
int recv_all(int socket, void *buffer, size_t length) {
    int bytes_read = 0, ret;
    char *ptr = (char *)buffer;
    do
    {
        ret = recv(socket, ptr + bytes_read, length - bytes_read, 0);
        bytes_read += ret;
    } while (ptr[bytes_read - 1] != '\0');

    return bytes_read;
}

static uid_t real_uid = 0;
static gid_t real_gid = 0;
static int privileges_initialized = 0;

void init_privileges() {
    const char *sudo_uid = getenv("SUDO_UID");
    const char *sudo_gid = getenv("SUDO_GID");

    if (sudo_uid && sudo_gid) {
        real_uid = (uid_t)atoi(getenv("SUDO_UID"));
        real_gid = (gid_t)atoi(sudo_gid);
        privileges_initialized = 1;
        printf("Privilege management initialized. Real UID: %d, GID: %d\n", real_uid, real_gid);
    } else {
        printf("Warning: SUDO_UID/SUDO_GID not found. Running as current user.\n");
    }
}

void minimize_privileges() {
    if (seteuid(real_uid) != 0) {
        perror("seteuid failed");
    }
}

void restore_privileges() {
    if (!privileges_initialized) return;

    if (seteuid(0) != 0) {
        perror("seteuid(0) failed");
    }
}
