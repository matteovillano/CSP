#include "../../include/utils.h"

void print_error(const char *msg) {
    perror(msg);
}

int send_all(int socket, char *buffer, int length) {
    int bytes_sent = 0, ret;
    char *ptr = buffer;
    while (bytes_sent < length)
    {
        ret = send(socket, ptr + bytes_sent, length - bytes_sent, 0);
        if (ret == -1 && errno == EINTR)
            continue;
        if (ret == -1){
            perror("send failed");
            exit(EXIT_FAILURE);
        }
        bytes_sent += ret;
    }
    return bytes_sent;
}

void send_string(int client_socket, char *str) {
    int length = strlen(str);
    send_all(client_socket, str, length + 1);
}

int recv_all(int socket, char *buffer, int length) {
    int bytes_read = 0, ret;
    char *ptr = buffer;
    do
    {
        ret = recv(socket, ptr + bytes_read, length - bytes_read, 0);
        if (ret == -1 && errno == EINTR) continue;
        if (ret == -1){
            perror("recv failed");
            exit(EXIT_FAILURE);
        }
        if (ret == 0) break;
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

uid_t get_real_uid() {
    return real_uid;
}
