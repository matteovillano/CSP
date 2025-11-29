#include <dirent.h> // For opendir and closedir
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h> // For stat and mkdir
#include <sys/types.h>
#include <unistd.h>
#include "../../include/network.h"
#include "../../include/utils.h"
#include "../../include/user_session.h"

int server_socket = -1;

int main(int argc, char *argv[]) {

    char *root_dir;
    char *ip = DEFAULT_IP;
    int port = DEFAULT_PORT;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <root_dir> [ip] [port]\n", argv[0]);
        exit(EXIT_FAILURE);
    } else if (argc > 4) {
        fprintf(stderr, "too many arguments!!\n");
        exit(EXIT_FAILURE);
    }

    root_dir = argv[1];
    if (argc >= 3) ip = argv[2];
    if (argc >= 4) port = atoi(argv[3]);

    // Initialize and drop privileges
    init_privileges();
    minimize_privileges();

    // create root directory if it doesn't exist
    struct stat st = {0};
    if (stat(root_dir, &st) == -1) {
        if (mkdir(root_dir, 0777) == -1) {
            perror("mkdir");
            exit(EXIT_FAILURE);
        }
        printf("Created root directory: %s\n", root_dir);
    }
    /*
    DIR *dir = opendir(root_dir);
    if (dir) {
        printf("Opened root directory: %s\n", root_dir);
        closedir(dir);
    } else {
        perror("opendir");
        exit(EXIT_FAILURE);
    }
    */
    // Create server socket
    server_socket = create_server_socket(port);
    if (server_socket == -1) {
        exit(EXIT_FAILURE);
    }

    printf("Server listening on %s:%d\n", ip, port);
    
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);

        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        printf("New connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            close(client_socket);
        } else if (pid == 0) {
            // Child process
            close(server_socket);
            handle_client(client_socket, root_dir);
            exit(0);
        } else {
            // Parent process
            close(client_socket);
        }
    }

    close(server_socket);

    return 0;
}
