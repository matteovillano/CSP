
#include "../../include/user_session.h"
#include "../../include/users.h"
#include "../../include/utils.h"

int handle_client(int client_socket, const char *root_dir) {

    char input[MAX_COMMAND_LENGTH];
    char command[MAX_COMMAND_LENGTH];
    char username[USERNAME_LENGTH];
    char permissions[5];

    printf("User Management Shell\n");

    while (1) {
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
            break;
        }

        int num_args = sscanf(input, "%s %s %s", command, username, permissions);

        if (num_args == 3) {
            if (strcmp(command, "create_user") == 0) {
                ret = create_user(username, strtol(permissions, NULL, 8), root_dir);
                if (!ret) {
                    printf("User %s created successfully.\n", username);
                    send_string(client_socket, "ok-create");
                } else {
                    send_string(client_socket, "err-create");
                }
            } 
        } else if (num_args == 2) {
            /* it is not specified in the assignment but it is a valid command */
            if (strcmp(command, "delete") == 0) {
                int id = get_id_by_username(username);
                if (id != -1) {
                    delete_user(id, root_dir);
                    send_string(client_socket, "ok-delete");
                } else {
                    send_string(client_socket, "err-delete");
                }
            } else if (strcmp(command, "login") == 0) {
                int id = get_id_by_username(username);
                if (id != -1) {
                    printf("User %s logged in successfully.\n", username);
                    send_string(client_socket, "ok-login");
                    user_session(client_socket, id, root_dir);
                    return 0;
                } else {
                    send_string(client_socket, "err-login");
                }
            }
        }else {
            send_string(client_socket, "err-invalid");
        }
        print_users();
    }
    return 0;
}