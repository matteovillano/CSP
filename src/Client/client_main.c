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
        if (!strcmp(buffer, "ok-login")){
            //printf("%s@server:~$ ", current_user);
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
        printf("%s@server:~$ ", current_user);
        // send command to server
        if (fgets(buffer, 256, stdin) == NULL)
            continue;

        // Intercept upload command
        if (strncmp(buffer, "upload", 6) == 0) {
            char cmd[10], arg1[128], arg2[128], arg3[128];
            int parsed = sscanf(buffer, "%s %s %s %s", cmd, arg1, arg2, arg3);
            
            int background = 0;
            char *local_path = NULL;
            char *remote_path = NULL;

            if (parsed >= 3) {
                if (strcmp(arg1, "-b") == 0) {
                    if (parsed < 4) {
                        printf("Usage: upload [-b] <local_path> <remote_path>\n");
                        continue;
                    }
                    background = 1;
                    local_path = arg2;
                    remote_path = arg3;
                } else {
                    local_path = arg1;
                    remote_path = arg2;
                }

                // Check if local file exists before doing anything
                if (access(local_path, F_OK) == -1) {
                    printf("Error: Local file '%s' does not exist.\n", local_path);
                    continue;
                }

                if (background) {
                    pid_t pid = fork();
                    if (pid == 0) {
                        // Child process
                        int child_socket = create_client_socket(server_ip, server_port);
                        if (child_socket < 0) exit(1);

                        // Login
                        char login_cmd[262];
                        snprintf(login_cmd, sizeof(login_cmd), "login %s", current_user);
                        send_string(child_socket, login_cmd);
                        
                        char resp[256];
                        recv_all(child_socket, resp, 255); // consume "ok-login"
                        
                        // Initiate upload
                        char upload_cmd[512];
                        snprintf(upload_cmd, sizeof(upload_cmd), "upload %s", remote_path);
                        send_string(child_socket, upload_cmd);
                        
                        recv_all(child_socket, resp, 255); // consume "ok-upload" or error
                        if (strncmp(resp, "ok-upload", 9) != 0) {
                            close(child_socket);
                            exit(1);
                        }

                        // Open local file
                        FILE *f = fopen(local_path, "rb");
                        if (!f) {
                            close(child_socket);
                            exit(1);
                        }
                        
                        fseek(f, 0, SEEK_END);
                        long fsize = ftell(f);
                        fseek(f, 0, SEEK_SET);
                        
                        // Send size
                        char size_str[64];
                        snprintf(size_str, sizeof(size_str), "%ld", fsize);
                        send_string(child_socket, size_str);
                        
                        recv_all(child_socket, resp, 255); // consume "ok-size"

                        // Send content
                        char *file_buf = malloc(4096);
                        size_t n;
                        while ((n = fread(file_buf, 1, 4096, f)) > 0) {
                            send(child_socket, file_buf, n, 0);
                        }
                        free(file_buf);
                        fclose(f);
                        
                        recv_all(child_socket, resp, 255); // consume "ok-concluded"
                        
                        printf("\n[Background] Command: upload %s %s concluded\n> ", remote_path, local_path);
                        fflush(stdout);
                        close(child_socket);
                        exit(0);
                    } else {
                        // Parent continues immediately
                        continue;
                    }
                } else {
                    // Foreground upload
                    // Send command to server
                    char upload_cmd[512];
                    snprintf(upload_cmd, sizeof(upload_cmd), "upload %s", remote_path);
                    if (send_all(client_socket, upload_cmd, strlen(upload_cmd) + 1) == 0) {
                        printf("error sending the message. Retry\n>");
                        continue;
                    }
                    
                    // Wait for ok-upload
                    memset(buffer, 0, 256);
                    if (recv_all(client_socket, buffer, 255) <= 0) continue;
                    
                    if (strcmp(buffer, "ok-upload") == 0) {
                        // Open local file
                        FILE *f = fopen(local_path, "rb");
                        if (!f) {
                            printf("Error opening local file\n");
                            continue;
                        }
                        
                        fseek(f, 0, SEEK_END);
                        long fsize = ftell(f);
                        fseek(f, 0, SEEK_SET);
                        
                        // Send size
                        char size_str[64];
                        snprintf(size_str, sizeof(size_str), "%ld", fsize);
                        send_string(client_socket, size_str);
                        
                        // Wait for ok-size
                        memset(buffer, 0, 256);
                        recv_all(client_socket, buffer, 255);
                        
                        if (strcmp(buffer, "ok-size") == 0) {
                             // Send content
                            char *file_buf = malloc(4096);
                            size_t n;
                            while ((n = fread(file_buf, 1, 4096, f)) > 0) {
                                send(client_socket, file_buf, n, 0);
                            }
                            free(file_buf);
                            fclose(f);
                            
                            // Wait for ok-concluded
                            memset(buffer, 0, 256);
                            recv_all(client_socket, buffer, 255);
                            printf("%s\n", buffer);
                        } else {
                            printf("Error: Expected ok-size, got %s\n", buffer);
                        }
                    } else {
                        printf("%s\n", buffer);
                    }
                    
                    // Skip the rest of the loop
                    continue;
                }
            }
        }

        if (send_all(client_socket, buffer, strlen(buffer) + 1) == 0) {
            printf("error sending the message. Retry\n>");
            continue;
        }
        
        // receive response from server
        memset(buffer, 0, 256);
        if (recv_all(client_socket, buffer, 255) <= 0)
            continue;

        if (!strcmp(buffer, "ok-write")) {
            printf("----Write content (max 4095 bytes)----\n");
            char content[4096];
            if (fgets(content, 4096, stdin) == NULL)
                continue;
            send_string(client_socket, content);
            memset(buffer, 0, 256);
            if (recv_all(client_socket, buffer, 255) <= 0)
                continue;
        }

        printf("%s\n", buffer);
        
        // manage responses
        if (!strcmp(buffer, "exit")){
            send_string(client_socket, "exit");
            return 0;
        }        

    }

    return 1;
}