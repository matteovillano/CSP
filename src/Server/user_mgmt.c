
#include "../../include/user_session.h"
#include "../../include/users.h"
#include "../../include/utils.h"

int handle_client(int client_socket, const char *root_dir) {

    char input[MAX_COMMAND_LENGTH];
    char command[MAX_COMMAND_LENGTH];
    char username[USERNAME_LENGTH];
    char permissions[5];

    printf("User Management Shell\n");
    printf("Commands: create <username>, delete <username>, login <username>, exit\n");

    while (1) {

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
                printf("Creating user: %s\n", username);
                create_user(username, strtol(permissions, NULL, 8), root_dir);
                printf("User %s created successfully.\n", username);
            } 
        } else if (num_args == 2) {
            if (strcmp(command, "delete") == 0) {
                int id = get_id_by_username(username);
                if (id != -1) {
                    delete_user(id, root_dir);
                } else {
                    printf("User %s not found.\n", username);
                }
            } else if (strcmp(command, "login") == 0) {
                int id = get_id_by_username(username);
                if (id != -1) {
                    user_session(client_socket, id);
                    return 0;
                } else {
                    printf("User %s not found.\n", username);
                }
            }
        }else {
            printf("Invalid command format. Use: create <username> <permissions> or delete <username>\n");
        }
        print_users();
    }
    return 0;
}