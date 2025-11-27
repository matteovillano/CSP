#include "../../include/network.h"
#include "../../include/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

char current_user[256] = "";
int client_socket = -1;
char server_ip[INET_ADDRSTRLEN];
int server_port;

int main(int argc, char *argv[]) {

    strncpy(server_ip, DEFAULT_IP, INET_ADDRSTRLEN);
    server_port = DEFAULT_PORT;

    if (argc > 3) {
        fprintf(stderr, "Too many arguments\n");
        exit(EXIT_FAILURE);
    }

    if (argc >= 2)
        strncpy(server_ip, argv[1], INET_ADDRSTRLEN);
    if (argc >= 3)
        server_port = atoi(argv[2]);

    // connect to server
    client_socket = create_client_socket(server_ip, server_port);
    if (client_socket < 0)
        exit(EXIT_FAILURE);

    printf("Connected to server at %s:%d\n", server_ip, server_port);
    printf("> ");

    char *buffer = calloc(256, 1);
    while (1)
    {
        if (fgets(buffer, 256, stdin) == NULL)
            continue;
        else
            if (send_all(client_socket, buffer, strlen(buffer) + 1) == 0)
                printf("send");
            else
                printf("error");

        printf("> ");
        //int bytes_received = recv_all(client_socket, buffer, sizeof(buffer) - 1);
    }
    

  return 0;
}