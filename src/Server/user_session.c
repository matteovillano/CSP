#include "../../include/user_session.h"
#include "../../include/ops.h"
#include "../../include/users.h"
#include "../../include/utils.h"
#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>

int user_session(int client_socket, int id, const char *root_dir) {
    char username[USERNAME_LENGTH];
    get_username_by_id(id, username);

    printf("------- User session - User: %s -------\n", username);

    // Switch to user identity
    struct passwd *pwd = getpwnam(username); // get user info
    if (pwd == NULL) {
        perror("getpwnam failed");
        return -1;
    }

    restore_privileges(); // Ensure we are root to change UID and chroot

     if (chroot(root_dir) != 0) {
        perror("chroot failed");
        return -1;
    }
    if (chdir("/") != 0) {
        perror("chdir / failed");
        return -1;
    }

    // Change to user's home directory (relative to new root)
    if (chdir(username) != 0) {
        perror("chdir username failed");
        return -1;
    }

    if (setgid(pwd->pw_gid) != 0) {
        perror("setgid failed");
        return -1;
    }
    if (setuid(pwd->pw_uid) != 0) {
        perror("setuid failed");
        return -1;
    }

    printf("Process identity switched to UID: %d, GID: %d\n", pwd->pw_uid, pwd->pw_gid);
    minimize_privileges();
    
    // Open current directory (which is now the user's home)
    DIR *dir;
    dir = opendir(".");
    if (dir == NULL) {
        printf("Error opening directory .\n");
        return -1;
    }
    printf("Directory opened successfully\n");

    while (1) {
        char input[MAX_COMMAND_LENGTH];
        char *args[5];
        int arg_count = 0;

        // receive command from client
        if (recv_all(client_socket, input, sizeof(input) - 1) <= 0)
            continue;
        
        // Remove trailing newline
        input[strcspn(input, "\n")] = 0;

        printf("Received: %s\n", input);

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
        printf("Command: %s\n", command);

        if (strcmp(command, "create") == 0) {
            op_create(client_socket, id, dir, &args[1], arg_count);
        }
        else if (strcmp(command, "chmod") == 0) {
            op_changemod(client_socket, id, dir, &args[1], arg_count);
        }
        else if (strcmp(command, "move") == 0) {
            op_move(client_socket, id, dir, &args[1], arg_count);
        }
        else if (strcmp(command, "upload") == 0) {
            op_upload(client_socket, id, dir, &args[1], arg_count);
        }
        else if (strcmp(command, "download") == 0) {
            op_download(client_socket, id, dir, &args[1], arg_count);
        }
        else if (strcmp(command, "cd") == 0) {
            op_cd(client_socket, id, dir, &args[1], arg_count);
        }
        else if (strcmp(command, "list") == 0) {
            op_list(client_socket, id, dir, &args[1], arg_count);
        }
        else if (strcmp(command, "read") == 0) {
            op_read(client_socket, id, dir, &args[1], arg_count);
        }
        else if (strcmp(command, "write") == 0) {
            op_write(client_socket, id, dir, &args[1], arg_count);
        }
        else if (strcmp(command, "delete") == 0) {
            op_del(client_socket, id, dir, &args[1], arg_count);
        }
        else {
            send_string(client_socket, "err-Invalid command");
        }
    }
    return 0;
}