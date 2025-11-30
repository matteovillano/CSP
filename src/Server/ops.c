#include "../../include/ops.h"
#include "../../include/utils.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int op_create(int client_socket, int id, DIR *dir, char *args[], int arg_count) {
    char msg[256];
    
    if (arg_count < 2) {
        sprintf(msg, "err-Usage: create <filename> <permissions> or create -d <dirname> <permissions>");
        send_string(client_socket, msg);
        return -1;
    }

    if (strcmp(args[0], "-d") == 0) {
        if (arg_count < 3) {
            send_string(client_socket, "err-Usage: create -d <dirname> <permissions>");
            return -1;
        }
        
        mode_t mode = (mode_t)strtol(args[2], NULL, 8);

        if (mkdirat(dirfd(dir), args[1], mode) == -1) {
            send_string(client_socket, "err-Error creating directory");
            perror("Error creating directory");
            return -1;
        }

        snprintf(msg, sizeof(msg), "ok-Directory %s created successfully with permissions %o.", args[1], mode);
        send_string(client_socket, msg);
    } else {
        mode_t mode = (mode_t)strtol(args[1], NULL, 8);

        int fd = openat(dirfd(dir), args[0], O_CREAT | O_WRONLY | O_EXCL, mode);
        if (fd == -1) {
            send_string(client_socket, "err-Error creating file");
            perror("Error creating file");
            return -1;
        }
        printf("File %s created successfully with permissions %o.\n", args[0], mode);

        close(fd);

        snprintf(msg, sizeof(msg), "ok-File %s created successfully with permissions %o.", args[0], mode);
        send_string(client_socket, msg);
    }
    return 0;
}

int op_changemod(int client_socket, int id, DIR *dir, char *args[], int arg_count) {
    printf("changemod\n");
    return 0;
}

int op_move(int client_socket, int id, DIR *dir, char *args[], int arg_count) {
    printf("move\n");
    return 0;
}

int op_upload(int client_socket, int id, DIR *dir, char *args[], int arg_count) {
    printf("upload\n");
    return 0;
}

int op_download(int client_socket, int id, DIR *dir, char *args[], int arg_count) {
    printf("download\n");
    return 0;
}

int op_cd(int client_socket, int id, DIR *dir, char *args[], int arg_count) {
    printf("cd\n");
    return 0;
}

int op_list(int client_socket, int id, DIR *dir, char *args[], int arg_count) {
    printf("list\n");
    return 0;
}

int op_read(int client_socket, int id, DIR *dir, char *args[], int arg_count) {
    printf("read\n");
    return 0;
}

int op_write(int client_socket, int id, DIR *dir, char *args[], int arg_count) {
    printf("write\n");
    return 0;
}
int op_del(int client_socket, int id, DIR *dir, char *args[], int arg_count) {
    printf("del\n");
    return 0;
}