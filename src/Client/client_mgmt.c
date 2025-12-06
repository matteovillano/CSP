#include "../../include/network.h"
#include "../../include/utils.h"
#include "../../include/client_mgmt.h"
#include "../../include/client_ops.h"

char current_user[256] = "";

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

            if (strncmp(buffer, "login", 5) == 0) {
                strncpy(current_user, buffer + 6, strlen(buffer) - 6);
                current_user[strlen(current_user)-1] = '\0';
            }
        }
        
        // receive response from server
        memset(buffer, 0, 256);
        if (recv_all(client_socket, buffer, 255) <= 0)
            continue;

        // if login is successful, break
        if (!strcmp(buffer, "ok-login successful")){
            break;
        } else if (!strcmp(buffer, "exit")) {
            return 0;
        }
        printf("%s\n> ", buffer);
    }

    return 1;
}

int client_session(char *buffer, int client_socket) {
    char recv_buffer[4096];
    while(1)
    {
        printf("%s@server:~$ ", current_user);
        // send command to server
        if (fgets(buffer, 256, stdin) == NULL)
            continue;

        // Intercept upload command
        if (strncmp(buffer, "upload", 6) == 0) {
            upload_client(buffer, client_socket);
            continue;
        } else if (strncmp(buffer, "read", 4) == 0) {
            read_client(buffer, client_socket);
            continue;
        } else if (strncmp(buffer, "download", 8) == 0) {
            download_client(buffer, client_socket);
            continue;
        }

        if (send_all(client_socket, buffer, strlen(buffer) + 1) == 0) {
            printf("error sending the message. Retry\n>");
            continue;
        }
        
        // receive response from server
        memset(recv_buffer, 0, 4096);
        if (recv_all(client_socket, recv_buffer, 4095) <= 0)
            continue;

        if (!strcmp(recv_buffer, "ok-write")) {
            printf("----Write content (max 4095 bytes)----\n");
            char content[4096];
            if (fgets(content, 4096, stdin) == NULL)
                continue;
            send_string(client_socket, content);
            memset(recv_buffer, 0, 4096);
            if (recv_all(client_socket, recv_buffer, 4095) <= 0)
                continue;
        }

        printf("%s\n", recv_buffer);
        
        // manage responses
        if (!strcmp(recv_buffer, "exit")){
            send_string(client_socket, "exit");
            return 0;
        }        

    }

    return 1;
}