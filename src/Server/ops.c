#include "../../include/ops.h"
#include "../../include/utils.h"
#include "../../include/users.h"

int op_create(int client_socket, int id, DIR *dir, char *args[], int arg_count) {
    (void)dir;
    char msg[256];
    
    if (arg_count < 2) {
        sprintf(msg, "err-Usage: create <filename> <permissions> or create -d <dirname> <permissions>");
        send_string(client_socket, msg);
        return -1;
    }

    // Identify target path
    char *target_path = (strcmp(args[0], "-d") == 0) ? args[1] : args[0];
    
    // Validate path
    char *path_copy = strdup(target_path);
    if (!path_copy) {
        send_string(client_socket, "err-Internal server error");
        return -1;
    }
    
    // Get parent directory
    char *parent = dirname(path_copy);
    char resolved_parent[PATH_MAX];
    
    // Resolve parent directory
    if (realpath(parent, resolved_parent) == NULL) {
        free(path_copy);
        send_string(client_socket, "err-Invalid path: Parent directory does not exist");
        return -1;
    }
    free(path_copy);

    // Get user home directory
    char username[USERNAME_LENGTH];
    if (get_username_by_id(id, username) != 0) {
        send_string(client_socket, "err-Internal server error");
        return -1;
    }
    char home_dir[PATH_MAX];
    snprintf(home_dir, sizeof(home_dir), "/%s", username);

    // Check if resolved parent is within home directory
    if (strncmp(resolved_parent, home_dir, strlen(home_dir)) != 0) {
        send_string(client_socket, "err-Access denied: Cannot create outside home directory");
        return -1;
    }

    if (strcmp(args[0], "-d") == 0) {
        if (arg_count < 3) {
            send_string(client_socket, "err-Usage: create -d <dirname> <permissions>");
            return -1;
        }
        
        mode_t mode = (mode_t)strtol(args[2], NULL, 8);

        // AT_FDCWD is a constant that represents the current working directory
        if (mkdirat(AT_FDCWD, args[1], mode) == -1) {
            send_string(client_socket, "err-Error creating directory");
            perror("Error creating directory");
            return -1;
        }

        snprintf(msg, sizeof(msg), "ok-Directory %s created successfully with permissions %o.", args[1], mode);
        send_string(client_socket, msg);
    } else {
        mode_t mode = (mode_t)strtol(args[1], NULL, 8);

        int fd = openat(AT_FDCWD, args[0], O_CREAT | O_WRONLY | O_EXCL, mode);
        if (fd == -1) {
            send_string(client_socket, "err-Error creating file");
            perror("Error creating file");
            return -1;
        }
        printf("File %s created successfully with permissions %o.\n", args[0], mode);

        close(fd);

        snprintf(msg, sizeof(msg), "ok-File %s created successfully with permissions %o.", args[0], mode);
        send_string(client_socket, msg);
    }
    return 0;
}

int op_changemod(int client_socket, int id, DIR *dir, char *args[], int arg_count) {
    printf("changemod\n");
    return 0;
}

int op_move(int client_socket, int id, DIR *dir, char *args[], int arg_count) {
    printf("move\n");
    return 0;
}

int op_upload(int client_socket, int id, DIR *dir, char *args[], int arg_count) {
    printf("upload\n");
    return 0;
}

int op_download(int client_socket, int id, DIR *dir, char *args[], int arg_count) {
    printf("download\n");
    return 0;
}

int op_cd(int client_socket, int id, DIR *dir, char *args[], int arg_count) {
    (void)dir;
    if (arg_count < 1) {
        send_string(client_socket, "err-Usage: cd <path>");
        return -1;
    }

    char *path = args[0];
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd failed");
        send_string(client_socket, "err-Internal server error");
        return -1;
    }

    // Get username for home directory check
    char username[USERNAME_LENGTH];
    if (get_username_by_id(id, username) != 0) {
        send_string(client_socket, "err-Internal server error");
        return -1;
    }
    char home_dir[1024];
    snprintf(home_dir, sizeof(home_dir), "/%s", username);

    // Resolve path
    // If absolute, must start with /username
    if (path[0] == '/') {
        if (strncmp(path, home_dir, strlen(home_dir)) != 0) {
            send_string(client_socket, "err-Access denied: Cannot go outside home directory");
            return -1;
        }
    }

    if (chdir(path) != 0) {
        perror("chdir failed");
        send_string(client_socket, "err-Invalid path");
        return -1;
    }

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd failed");
        send_string(client_socket, "err-Internal server error");
        return -1;
    }

    // Verify we are still inside home_dir
    if (strncmp(cwd, home_dir, strlen(home_dir)) != 0) {
        // Revert
        chdir(home_dir);
        send_string(client_socket, "err-Access denied: Cannot go outside home directory");
        return -1;
    }
    
    send_string(client_socket, "ok-Changed directory");
    return 0;
}

int op_list(int client_socket, int id, DIR *dir, char *args[], int arg_count) {
    (void)id; (void)dir;
    char *path = ".";
    if (arg_count > 0) {
        path = args[0];
    }

    DIR *d = opendir(path);
    if (d == NULL) {
        perror("opendir failed");
        send_string(client_socket, "err-Error opening directory");
        return -1;
    }

    struct dirent *entry;
    char buffer[4096] = "";
    while ((entry = readdir(d)) != NULL) {
        strncat(buffer, entry->d_name, sizeof(buffer) - strlen(buffer) - 1);
        strncat(buffer, "\n", sizeof(buffer) - strlen(buffer) - 1);
    }
    closedir(d);

    // Send the list
    char msg[4100];
    snprintf(msg, sizeof(msg), "%s", buffer);
    send_string(client_socket, msg);
    return 0;
}

int op_read(int client_socket, int id, DIR *dir, char *args[], int arg_count) {
    printf("read\n");
    return 0;
}

int op_write(int client_socket, int id, DIR *dir, char *args[], int arg_count) {
    printf("write\n");
    return 0;
}
int op_del(int client_socket, int id, DIR *dir, char *args[], int arg_count) {
    printf("del\n");
    return 0;
}