
#include "../../include/user_session.h"
#include "../../include/users.h"
#include "../../include/utils.h"

int handle_client(int client_socket, const char *root_dir) {
    
    char input[MAX_COMMAND_LENGTH];

    printf("------- User Management Shell -------\n");
    
    while (1) {
        
        char *args[3];
        int ret;

        int bytes_received = recv_all(client_socket, input, sizeof(input) - 1);
        if (bytes_received <= 0) {
            printf("Client disconnected or error occurred.\n");
            break;
        }
        
        // Remove trailing newline
        input[strcspn(input, "\n")] = 0;
        
        printf("Received command: %s\n", input);

        if (strcmp(input, "exit") == 0) {
            send_string(client_socket, "exit");
            break;
        }

        // Tokenize input, args[0] = command, args[1] = username, args[2] = permission
        int arg_count = 0;
        char *token = strtok(input, " ");
        while (token != NULL) {
            if (arg_count >= 3) break;
            args[arg_count++] = token;
            token = strtok(NULL, " ");
        }

        // if there are too many arguments, skip
        if (token != NULL) {
            send_string(client_socket, "err-invalid command");
            continue;
        }

        if (arg_count == 3) {
            if (strcmp(args[0], "create_user") == 0) {
                if (atoi(args[2]) < 0 || atoi(args[2]) > 777) {
                    send_string(client_socket, "err-permission not valid");
                    continue;
                }
                mode_t perm = (mode_t)strtol(args[2], NULL, 8);
                ret = create_user(args[1], perm, root_dir);
                if (!ret) {
                    printf("User %s created successfully.\n", args[1]);
                    send_string(client_socket, "ok-created the user! You can now login");
                } else {
                    send_string(client_socket, "err-creating the user");
                }
            } else {
                send_string(client_socket, "err-invalid command");
            }
        } else if (arg_count == 2) {
            int id = get_id_by_username(args[1]);
            /* it is not specified in the assignment but it is a valid command */
            if (strcmp(args[0], "delete") == 0) {
                if (id != -1) {
                    delete_user(id, root_dir);
                    send_string(client_socket, "ok-user deleted");
                } else {
                    send_string(client_socket, "err-user not found");
                }
            } else if (strcmp(args[0], "login") == 0) {
                if (id != -1) {
                    printf("User %s logged in successfully.\n", args[1]);
                    send_string(client_socket, "ok-login successful");
                    user_session(client_socket, id, root_dir);
                    return 0;
                } else {
                    send_string(client_socket, "err-user not found");
                }
            } else {
                send_string(client_socket, "err-invalid command");
            }
        }else {
            send_string(client_socket, "err-invalid command");
        }
        print_users();
        memset(input, 0, sizeof(input));
    }
    return 0;
}