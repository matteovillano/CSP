#include "../../include/client_ops.h"
#include "../../include/network.h"
#include "../../include/utils.h"
#include "../../include/client_mgmt.h"

extern char server_ip[INET_ADDRSTRLEN];
extern int server_port;
extern char current_user[256];
extern int client_socket;

void download_client(char* buffer, int client_socket) {
    char cmd[10], arg1[128], arg2[128], arg3[128];
    int parsed = sscanf(buffer, "%s %s %s %s", cmd, arg1, arg2, arg3);
    
    int background = 0;
    char *remote_path = NULL;
    char *local_path = NULL;

    if (parsed >= 3) {
        if (strcmp(arg1, "-b") == 0) {
            if (parsed < 4) {
                printf("Usage: download [-b] <remote_path> <local_path>\n");
                return;
            }
            background = 1;
            remote_path = arg2;
            local_path = arg3;
        } else {
            remote_path = arg1;
            local_path = arg2;
        }

        if (background) {
            pid_t pid = fork();
            if (pid == 0) {
                sleep(5); // to simulate the background process

                // Child process
                int child_socket = create_client_socket(server_ip, server_port);
                if (child_socket < 0) exit(1);

                // Login
                char login_cmd[262];
                snprintf(login_cmd, sizeof(login_cmd), "login %s", current_user);
                send_string(child_socket, login_cmd);
                
                char resp[256];
                recv_all(child_socket, resp, 255); // consume "ok-login"
                
                // Initiate download
                char download_cmd[512];
                snprintf(download_cmd, sizeof(download_cmd), "download %s", remote_path);
                send_string(child_socket, download_cmd);
                
                recv_all(child_socket, resp, 255); // consume "ok-download" or error
                if (strncmp(resp, "ok-download", 11) != 0) {
                    printf("\n%s\n%s@server:~$ ", resp, current_user);
                    close(child_socket);
                    exit(1);
                }

                // Receive size
                char size_str[64];
                recv_all(child_socket, size_str, 63);
                long fsize = atol(size_str);
                
                // Send confirmation
                send_string(child_socket, "ok-size");
                
                // Open local file
                FILE *f = fopen(local_path, "wb");
                if (!f) {
                    close(child_socket);
                    exit(1);
                }

                // Receive content
                char *file_buf = malloc(4096);
                long total_received = 0;
                while (total_received < fsize) {
                    int chunk_size = 4096;
                    if (fsize - total_received < 4096) {
                        chunk_size = fsize - total_received;
                    }
                    
                    int bytes_read = recv(child_socket, file_buf, chunk_size, 0);
                    if (bytes_read <= 0) break;
                    
                    fwrite(file_buf, 1, bytes_read, f);
                    total_received += bytes_read;
                }
                
                free(file_buf);
                fclose(f);

                recv_all(child_socket, resp, 255); // consume "ok-concluded"
                
                printf("\n[Background] Command: download %s %s concluded\n%s@server:~$ ", remote_path, local_path, current_user);
                fflush(stdout);
                
                close(child_socket);
                exit(0);
            } else {
                // Parent continues immediately
                return;
            }
        } else {
            // Foreground download
            char download_cmd[512];
            snprintf(download_cmd, sizeof(download_cmd), "download %s", remote_path);
            if (send_all(client_socket, download_cmd, strlen(download_cmd) + 1) == 0) {
                printf("error sending the message. Retry\n>");
                return;
            }
            
            // Wait for ok-download
            memset(buffer, 0, 256);
            if (recv_all(client_socket, buffer, 255) <= 0) return;
            
            if (strcmp(buffer, "ok-download") == 0) {
                // Receive size
                char size_str[64];
                recv_all(client_socket, size_str, 63);
                long fsize = atol(size_str);
                
                // Send confirmation
                send_string(client_socket, "ok-size");
                
                // Open local file
                FILE *f = fopen(local_path, "wb");
                if (!f) {
                    printf("Error opening local file\n");
                    return;
                }
                
                // Receive content
                char *file_buf = malloc(4096);
                long total_received = 0;
                while (total_received < fsize) {
                    int chunk_size = 4096;
                    if (fsize - total_received < 4096) {
                        chunk_size = fsize - total_received;
                    }
                    
                    int bytes_read = recv(client_socket, file_buf, chunk_size, 0);
                    if (bytes_read <= 0) break;
                    
                    fwrite(file_buf, 1, bytes_read, f);
                    total_received += bytes_read;
                }
                
                free(file_buf);
                fclose(f);
                
                // Wait for ok-concluded
                memset(buffer, 0, 256);
                recv_all(client_socket, buffer, 255);
                printf("file downloaded\n");
            } else {
                printf("%s\n", buffer);
            }
            
            return;
        }
    } else {
        printf("Usage: download [-b] <remote_path> <local_path>\n");
        return;
    }
}

void upload_client(char* buffer, int client_socket) {
    char cmd[10], arg1[128], arg2[128], arg3[128];
    int parsed = sscanf(buffer, "%s %s %s %s", cmd, arg1, arg2, arg3);
    
    int background = 0;
    char *local_path = NULL;
    char *remote_path = NULL;

    if (parsed >= 3) {
        if (strcmp(arg1, "-b") == 0) {
            if (parsed < 4) {
                printf("Usage: upload [-b] <local_path> <remote_path>\n");
                return;
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
            return;
        }

        if (background) {
            pid_t pid = fork();
            if (pid == 0) {
                sleep(5); // to simulate the background process

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
                    printf("\n%s\n%s@server:~$ ", resp, current_user);
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
                    send_all(child_socket, file_buf, n);
                }
                
                free(file_buf);
                fclose(f);

                recv_all(child_socket, resp, 255); // consume "ok-concluded"

                printf("\n[Background] Command: upload %s %s concluded\n%s@server:~$ ", remote_path, local_path, current_user);
                fflush(stdout);
                
                close(child_socket);
                exit(0);
            } else {
                // Parent continues immediately
                return;
            }
        } else {
            char resp[256];
            // Foreground upload
            // Send command to server
            char upload_cmd[512];
            snprintf(upload_cmd, sizeof(upload_cmd), "upload %s", remote_path);
            if (send_all(client_socket, upload_cmd, strlen(upload_cmd) + 1) == 0) {
                printf("error sending the message. Retry\n>");
                return;
            }
            
            // Wait for ok-upload
            memset(buffer, 0, 256);
            if (recv_all(client_socket, buffer, 255) <= 0) return;
            
            if (strcmp(buffer, "ok-upload") == 0) {
                // Open local file
                FILE *f = fopen(local_path, "rb");
                if (!f) {
                    printf("Error opening local file\n");
                    return;
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
                    while (fsize > 0) {
                        n = fread(file_buf, 1, 4096, f);
                        send(client_socket, file_buf, n, 0);
                        fsize -= n;
                    }
                    free(file_buf);
                    fclose(f);

                    recv_all(client_socket, resp, 255); // consume "ok-concluded"
                    printf("upload concluded\n");
                } else {
                    printf("Error: Expected ok-size, got %s\n", buffer);
                }
            } else {
                printf("%s\n", buffer);
            }
            
            // Skip the rest of the loop
            return;
        }
    } else {
        printf("Usage: upload [-b] <local_path> <remote_path>\n");
        return;
    }
}

void read_client(char* buffer, int client_socket) {
    if (send_all(client_socket, buffer, strlen(buffer) + 1) == 0) {
        printf("error sending the message. Retry\n>");
        return;
    }

    // receive response from server
    memset(buffer, 0, 256);

    char size[32];
    recv_all(client_socket, size, sizeof(size));
    int file_length = atoi(size);

    send_string(client_socket, "ok-size");

    char resp[4096];
    while (file_length > 0) {
        int n = recv_all(client_socket, resp, sizeof(resp));
        if (strncmp(resp, "err", 3) == 0) {
            return;
        }
        printf("%s\n", resp);
        memset(resp, 0, sizeof(resp));
        file_length -= n;
    }

    recv_all(client_socket, resp, sizeof(resp)); // consume "ok-concluded"
    printf("\nfile read\n");
}
