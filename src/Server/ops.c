#include "../../include/ops.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int op_create(int id, DIR *dir, char *args[], int arg_count) {
    if (arg_count < 2) {
        printf("Usage: create <filename> <permissions> or create -d <dirname> "
           "<permissions>\n");
        return -1;
    }

    if (strcmp(args[0], "-d") == 0) {
        if (arg_count < 3) {
            printf("Usage: create -d <dirname> <permissions>\n");
            return -1;
        }
        char *endptr;
        long val = strtol(args[2], &endptr, 8);
        if (*endptr != '\0' || val < 0 || val > 0777) {
            printf("Invalid permissions: %s\n", args[2]);
            return -1;
        }
        mode_t mode = (mode_t)val;

        if (mkdirat(dirfd(dir), args[1], mode) == -1) {
            perror("Error creating directory");
            return -1;
        }
        printf("Directory %s created successfully with permissions %o.\n", args[1],
           mode);
    } else {
        char *endptr;
        long val = strtol(args[1], &endptr, 8);
        if (*endptr != '\0' || val < 0 || val > 0777) {
            printf("Invalid permissions: %s\n", args[1]);
            return -1;
        }
        mode_t mode = (mode_t)val;

        int fd = openat(dirfd(dir), args[0], O_CREAT | O_WRONLY | O_EXCL, mode);
        if (fd == -1) {
            perror("Error creating file");
            return -1;
        }
        close(fd);
        printf("File %s created successfully with permissions %o.\n", args[0],
           mode);
    }
    return 0;
}

int op_changemod(int id, DIR *dir, char *args[], int arg_count) {
    printf("changemod\n");
    return 0;
}

int op_move(int id, DIR *dir, char *args[], int arg_count) {
    printf("move\n");
    return 0;
}

int op_upload(int id, DIR *dir, char *args[], int arg_count) {
    printf("upload\n");
    return 0;
}

int op_download(int id, DIR *dir, char *args[], int arg_count) {
    printf("download\n");
    return 0;
}

int op_cd(int id, DIR *dir, char *args[], int arg_count) {
    printf("cd\n");
    return 0;
}

int op_list(int id, DIR *dir, char *args[], int arg_count) {
    printf("list\n");
    return 0;
}

int op_read(int id, DIR *dir, char *args[], int arg_count) {
    printf("read\n");
    return 0;
}

int op_write(int id, DIR *dir, char *args[], int arg_count) {
    printf("write\n");
    return 0;
}
int op_del(int id, DIR *dir, char *args[], int arg_count) {
    printf("del\n");
    return 0;
}