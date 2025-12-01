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

int create_and_login_user(char* buffer, int client_socket);
int client_session(char* buffer, int client_socket);

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
    printf("Commands: create <username> <permissions>, delete <username>, login <username>, exit\n");
    printf("> ");

    char buffer[256];
    // creation and login user
    if (!create_and_login_user(buffer, client_socket)) {
        printf("Exiting\n");
        return 0;
    }
    
    // user session
    if (!client_session(buffer, client_socket)) {
        printf("Exiting\n");
        return 0;
    }

    return 0;
}

int create_and_login_user(char *buffer, int client_socket) {
    while (1)
    {
        memset(buffer, 0, 256);
        // send command to server
        if (fgets(buffer, 256, stdin) == NULL)
            continue;
        else {
            if (send_all(client_socket, buffer, strlen(buffer) + 1) == 0) {
                printf("error sending the message. Retry\n>");
                continue;
            }

            if (strcmp(buffer, "login") > 0) {
                strncpy(current_user, buffer + 6, strlen(buffer) - 6);
                current_user[strlen(current_user)-1] = '\0';
            }
        }
        
        // receive response from server
        memset(buffer, 0, 256);
        int bytes_received = recv_all(client_socket, buffer, 255);

        // if login is successful, break
        if (!strcmp(buffer, "ok-login")){
            printf("%s@server:~$ ", current_user);
            break;
        } else if (!strcmp(buffer, "exit")) {
            return 0;
        }
        printf("%s\n> ", buffer);
    }

    return 1;
}

int client_session(char *buffer, int client_socket) {
    while(1)
    {
        // send command to server
        if (fgets(buffer, 256, stdin) == NULL)
            continue;
        else
            if (send_all(client_socket, buffer, strlen(buffer) + 1) == 0) {
                printf("error sending the message. Retry\n>");
                continue;
            }
        
        // receive response from server
        memset(buffer, 0, 256);
        int bytes_received = recv_all(client_socket, buffer, 255);

        printf("%s\n", buffer);
        
        // manage responses
        if (bytes_received > 0) {
            if (strcmp(buffer, "exit") == 0){
                send_string(client_socket, "exit");
                return 0;
            }
        }
        
        printf("%s@server:~$ ", current_user);

    }

    return 1;
}