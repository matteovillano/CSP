#include "../../include/user_session.h"
#include "../../include/ops.h"
#include "../../include/users.h"
#include "../../include/utils.h"

int user_session(int client_socket, int id, const char *root_dir) {
    char username[USERNAME_LENGTH];
    get_username_by_id(id, username);

    printf("------- User session --------\nUser: %s\n", username);
    
    char path[256];
    snprintf(path, sizeof(path), "%s/%s", root_dir, username);

    DIR *dir;
    dir = opendir(path);
    if (dir == NULL) {
        printf("Error opening directory %s\n", path);
        return -1;
    }
    /* it is used to print the files in the directory, not necessary
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        printf("%s\n", entry->d_name);
    }
    closedir(dir);*/

    while (1) {
        char input[MAX_COMMAND_LENGTH];
        char *args[5];
        int arg_count = 0;

        // receive command from client
        int bytes_received = recv_all(client_socket, input, sizeof(input) - 1);

        // Remove trailing newline
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "exit") == 0) {
            break;
        }

        // Tokenize input
        char *token = strtok(input, " ");
        while (token != NULL) {
            args[arg_count++] = token;
            token = strtok(NULL, " ");
        }

        if (arg_count == 0) {
            continue;
        }

        char *command = args[0];
        arg_count--;

        if (strcmp(command, "create") == 0) {
            op_create(id, dir, &args[1], arg_count);
        }
        if (strcmp(command, "chmod") == 0) {
            op_changemod(id, dir, &args[1], arg_count);
        }
        if (strcmp(command, "move") == 0) {
            op_move(id, dir, &args[1], arg_count);
        }
        if (strcmp(command, "upload") == 0) {
            op_upload(id, dir, &args[1], arg_count);
        }
        if (strcmp(command, "download") == 0) {
            op_download(id, dir, &args[1], arg_count);
        }
        if (strcmp(command, "cd") == 0) {
            op_cd(id, dir, &args[1], arg_count);
        }
        if (strcmp(command, "list") == 0) {
            op_list(id, dir, &args[1], arg_count);
        }
        if (strcmp(command, "read") == 0) {
            op_read(id, dir, &args[1], arg_count);
        }
        if (strcmp(command, "write") == 0) {
            op_write(id, dir, &args[1], arg_count);
        }
        if (strcmp(command, "delete") == 0) {
            op_del(id, dir, &args[1], arg_count);
        }
    }
    return 0;
}