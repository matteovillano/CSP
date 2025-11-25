#include "../../include/network.h"
#include <dirent.h> // For opendir and closedir
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/stat.h> // For stat and mkdir
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

    char *root_dir;
    char *ip = DEFAULT_IP;
    int port = DEFAULT_PORT;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <root_dir> [ip] [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    root_dir = argv[1];
    if (argc >= 3) ip = argv[2];
    if (argc >= 4) port = atoi(argv[3]);

    struct stat st = {0};
    if (stat(root_dir, &st) == -1) {
        if (mkdir(root_dir, 0777) == -1) {
            perror("mkdir");
            exit(EXIT_FAILURE);
        }
        printf("Created root directory: %s\n", root_dir);
    }

    DIR *dir = opendir(root_dir);
    if (dir) {
        printf("Opened root directory: %s\n", root_dir);
        closedir(dir);
    } else {
        perror("opendir");
        exit(EXIT_FAILURE);
    }

    printf("ip: %s\n", ip);
    printf("port: %d\n", port);
    printf("root_dir: %s\n", root_dir);

    return 0;
}
