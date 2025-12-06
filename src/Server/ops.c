#include "../../include/ops.h"
#include "../../include/utils.h"
#include "../../include/users.h"
#include "../../include/concurrency.h"
#include <libgen.h>

void get_full_path(char *path, char *full_path);
int check_path(int client_socket, int id, char *path);

int op_create(int client_socket, int id, DIR *dir, char *args[], int arg_count) {
    (void)dir;
    char msg[256];
    
    if (arg_count < 2) {
        send_string(client_socket, "err-Usage: create <filename> <permissions> or create -d <dirname> <permissions>");
        return -1;
    }

    // Identify target path
    char *target_path = (strcmp(args[0], "-d") == 0) ? args[1] : args[0];
    
    // Validate path
    if (check_path(client_socket, id, target_path) != 0) {
        return -1;
    }

    if (strcmp(args[0], "-d") == 0) {
        if (arg_count < 3) {
            send_string(client_socket, "err-Usage: create -d <dirname> <permissions>");
            return -1;
        }
        if (atoi(args[2]) < 0 || atoi(args[2]) > 777) {
            send_string(client_socket, "err-permission not valid");
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
        if (atoi(args[1]) < 0 || atoi(args[1]) > 777) {
            send_string(client_socket, "err-permission not valid");
            return -1;
        }
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
    (void)dir;
    char msg[256];

    if (arg_count < 2) {
        send_string(client_socket, "err-Usage: chmod <path> <permissions>");
        return -1;
    }

    if (atoi(args[1]) < 0 || atoi(args[1]) > 777) {
        send_string(client_socket, "err-permission not valid");
        return -1;
    }

    // Get user home directory
    char username[USERNAME_LENGTH];
    if (get_username_by_id(id, username) != 0) {
        send_string(client_socket, "err-Internal server error");
        return -1;
    }
    char home_dir[PATH_MAX];
    snprintf(home_dir, sizeof(home_dir), "/%s", username);

    int home_fd = open(home_dir, O_RDONLY | O_DIRECTORY);
    if (home_fd == -1) {
        perror("open home failed");
        send_string(client_socket, "err-Internal server error");
        return -1;
    }

    if (check_path(client_socket, id, args[0]) != 0) {
        return -1;
    }

    close(home_fd);

    mode_t mode = (mode_t)strtol(args[1], NULL, 8);
    char *path_copy2 = strdup(args[0]);
 
    if (fchmodat(AT_FDCWD, args[0], mode, 0) == -1) {
        perror("chmod failed");
        send_string(client_socket, "err-Error changing permissions");
        free(path_copy2);
        return -1;
    }

    free(path_copy2);

    snprintf(msg, sizeof(msg), "ok-Permissions for %s changed to %o.", args[0], mode);
    send_string(client_socket, msg);
    return 0;
}

int op_move(int client_socket, int id, DIR *dir, char *args[], int arg_count) {
    (void)dir;
    char msg[256];

    if (arg_count < 2) {
        send_string(client_socket, "err-Usage: move <source> <destination>");
        return -1;
    }

    // Validate source path
    if (check_path(client_socket, id, args[0]) != 0) {
        return -1;
    }

    // Validate destination path
    if (check_path(client_socket, id, args[1]) != 0) {
        return -1;
    }

    if (rename(args[0], args[1]) == -1) {
        perror("rename failed");
        send_string(client_socket, "err-Error moving file");
        return -1;
    }

    snprintf(msg, sizeof(msg), "ok-Moved %s to %s.", args[0], args[1]);
    send_string(client_socket, msg);
    return 0;
}

int op_upload(int client_socket, int id, DIR *dir, char *args[], int arg_count) {
    (void)dir;
    char file_path[256];
    long file_size = 0;

    if (arg_count < 1) {
        send_string(client_socket, "err-Usage: upload <path>");
        return -1;
    }

    strncpy(file_path, args[0], sizeof(file_path) - 1);
    file_path[sizeof(file_path) - 1] = '\0';

    // Validate path
    if (check_path(client_socket, id, file_path) != 0) {
        send_string(client_socket, "err-Invalid path");
        return -1;
    }

    char full_path[PATH_MAX];
    get_full_path(file_path, full_path);
    FileLock *lock = get_file_lock(full_path);
    if (!lock) {
        send_string(client_socket, "err-Server busy (too many locks)");
        return -1;
    }
    writer_lock(lock);

    // Open file for writing (create or truncate)
    int fd = openat(AT_FDCWD, file_path, O_WRONLY | O_CREAT | O_TRUNC, 0700);
    if (fd == -1) {
        perror("open failed");
        send_string(client_socket, "err-Error opening/creating file");
        writer_unlock(lock);
        release_file_lock(lock);
        return -1;
    }

    // Send ready signal
    send_string(client_socket, "ok-upload");

    // Receive file size
    char size_buffer[64];
    if (recv_all(client_socket, size_buffer, 63) <= 0) {
        close(fd);
        writer_unlock(lock);
        release_file_lock(lock);
        return -1;
    }
    file_size = atol(size_buffer);
    
    // Send size received confirmation
    send_string(client_socket, "ok-size");

    // Receive content
    char *buffer = malloc(4096);
    if (!buffer) {
        close(fd);
        writer_unlock(lock);
        release_file_lock(lock);
        return -1;
    }

    long total_received = 0;
    while (total_received < file_size) {
        int chunk_size = 4096;
        if (file_size - total_received < 4096) {
            chunk_size = file_size - total_received;
        }
        
        int bytes_read = recv(client_socket, buffer, chunk_size, 0);
        if (bytes_read <= 0) {
            perror("recv failed during upload");
            break;
        }
        
        if (write(fd, buffer, bytes_read) != bytes_read) {
            perror("write failed");
            break;
        }
        total_received += bytes_read;
    }

    free(buffer);
    close(fd);
    writer_unlock(lock);
    release_file_lock(lock);

    if (total_received == file_size) {
        send_string(client_socket, "ok-concluded");
        return 0;
    } else {
        return -1;
    }
}

int op_download(int client_socket, int id, DIR *dir, char *args[], int arg_count) {
    (void)dir;
    char file_path[256];
    struct stat file_stat;

    if (arg_count < 1) {
        send_string(client_socket, "err-Usage: download <path>");
        return -1;
    }

    strncpy(file_path, args[0], sizeof(file_path) - 1);
    file_path[sizeof(file_path) - 1] = '\0';

    // Validate path
    if (check_path(client_socket, id, file_path) != 0) {
        send_string(client_socket, "err-Invalid path");
        return -1;
    }

    char full_path[PATH_MAX];
    get_full_path(file_path, full_path);
    
    FileLock *lock = get_file_lock(full_path);
    if (!lock) {
        send_string(client_socket, "err-Server busy (too many locks)");
        return -1;
    }
    reader_lock(lock);

    // Check if file exists and get info
    if (stat(file_path, &file_stat) == -1) {
        perror("stat failed");
        send_string(client_socket, "err-File not found");
        reader_unlock(lock);
        release_file_lock(lock);
        return -1;
    }

    if (!S_ISREG(file_stat.st_mode)) {
        send_string(client_socket, "err-Not a regular file");
        reader_unlock(lock);
        release_file_lock(lock);
        return -1;
    }

    int fd = openat(AT_FDCWD, file_path, O_RDONLY);
    if (fd == -1) {
        perror("open failed");
        send_string(client_socket, "err-Error opening file");
        reader_unlock(lock);
        release_file_lock(lock);
        return -1;
    }

    // Send ready signal
    send_string(client_socket, "ok-download");

    // Send file size
    char size_str[64];
    snprintf(size_str, sizeof(size_str), "%ld", file_stat.st_size);
    send_string(client_socket, size_str);

    // Wait for client confirmation
    char buffer[256];
    if (recv_all(client_socket, buffer, 255) <= 0) {
        close(fd);
        reader_unlock(lock);
        release_file_lock(lock);
        return -1;
    }
    
    if (strcmp(buffer, "ok-size") != 0) {
        close(fd);
        reader_unlock(lock);
        release_file_lock(lock);
        return -1;
    }

    // Send content
    char *file_buf = malloc(4096);
    if (!file_buf) {
        close(fd);
        reader_unlock(lock);
        release_file_lock(lock);
        return -1;
    }

    ssize_t bytes_read;
    while ((bytes_read = read(fd, file_buf, 4096)) > 0) {
        if (send_all(client_socket, file_buf, bytes_read) != bytes_read) {
            perror("send failed");
            break;
        }
    }

    free(file_buf);
    close(fd);
    reader_unlock(lock);
    release_file_lock(lock);

    // Send conclusion message
    send_string(client_socket, "ok-concluded");
    
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
    (void)dir;
    char *path = ".";
    if (arg_count > 0) {
        path = args[0];
        if (check_path(client_socket, id, path) != 0) {
            return -1;
        }
    }

    DIR *d = opendir(path);
    if (d == NULL) {
        perror("opendir failed");
        send_string(client_socket, "err-Error opening directory");
        return -1;
    }

    struct dirent *entry;
    char buffer[4096] = "";
    struct stat file_stat;
    char full_path[PATH_MAX];

    while ((entry = readdir(d)) != NULL) {

        get_full_path(path, full_path);
        if (stat(full_path, &file_stat) == -1) {
            perror("stat failed");
            continue;
        }

        char line[512];
        snprintf(line, sizeof(line), "%s %o %ld\n", entry->d_name, file_stat.st_mode & 0777, file_stat.st_size);
        
        if (strlen(buffer) + strlen(line) < sizeof(buffer) - 1) {
            strcat(buffer, line);
        } else {
            break;
        }
    }
    closedir(d);

    if (strlen(buffer) == 0) {
         send_string(client_socket, "");
    } else {
         send_string(client_socket, buffer);
    }
    return 0;
}

int op_read(int client_socket, int id, DIR *dir, char *args[], int arg_count) {
    (void)dir;
    char msg[256];
    char file_path[256];
    int offset = 0;
    
    if (arg_count < 1) {
        send_string(client_socket, "err-Usage: read [-offset=<num>] <path>");
        return -1;
    }
    
    // Parse arguments
    if (strncmp(args[0], "-offset=", 8) == 0) {
        if (arg_count < 2) {
            send_string(client_socket, "err-Usage: read [-offset=<num>] <path>");
            return -1;
        }
        offset = atoi(args[0] + 8);
        strncpy(file_path, args[1], sizeof(file_path) - 1);
        file_path[sizeof(file_path) - 1] = '\0';
    } else {
        strncpy(file_path, args[0], sizeof(file_path) - 1);
        file_path[sizeof(file_path) - 1] = '\0';
    }
    
    // Validate path
    if (check_path(client_socket, id, file_path) != 0) {
        return -1;
    }

    char full_path[PATH_MAX];
    get_full_path(file_path, full_path);
    FileLock *lock = get_file_lock(full_path);
    if (!lock) {
        send_string(client_socket, "err-Server busy (too many locks)");
        return -1;
    }
    reader_lock(lock);
    
    // Open file
    int fd = openat(AT_FDCWD, file_path, O_RDONLY);
    if (fd == -1) {
        perror("open failed");
        send_string(client_socket, "err-Error opening file");
        reader_unlock(lock);
        release_file_lock(lock);
        return -1;
    }

    // control the file length
    int file_length = lseek(fd, 0, SEEK_END);
    if (file_length == -1) {
        perror("lseek failed");
        close(fd);
        send_string(client_socket, "err-Error getting file length");
        reader_unlock(lock);
        release_file_lock(lock);
        return -1;
    }
    
    // Seek to the offset
    if (offset > 0) {
        if (lseek(fd, offset, SEEK_SET) == -1) {
            perror("lseek failed");
            close(fd);
            send_string(client_socket, "err-Error seeking file");
            reader_unlock(lock);
            release_file_lock(lock);
            return -1;
        }
    } else {
        if (lseek(fd, 0, SEEK_SET) == -1) {
            perror("lseek failed");
            close(fd);
            send_string(client_socket, "err-Error seeking file");
            reader_unlock(lock);
            release_file_lock(lock);
            return -1;
        }
    }

    char size[32];
    snprintf(size, sizeof(size), "%d", file_length);
    send_string(client_socket, size);

    recv_all(client_socket, msg, sizeof(msg)); // consume ok-size
    
    // Read content
    char content[4096];
    int bytes_read = 0;
    // send 4096 bytes at a time
    while (file_length > 0) {
        bytes_read = read(fd, content, sizeof(content)-1);
        if (bytes_read == -1) {
            perror("read failed");
            close(fd);
            send_string(client_socket, "err-Error reading file");
            reader_unlock(lock);
            release_file_lock(lock);
            return -1;
        }
        content[bytes_read] = '\0';
        send_string(client_socket, content);
        file_length -= bytes_read;
    }

    printf("File read\n");
    send_string(client_socket, "ok-concluded");

    close(fd);
    reader_unlock(lock);
    release_file_lock(lock);
    
    return 0;
}

int op_write(int client_socket, int id, DIR *dir, char *args[], int arg_count) {
    (void)dir;
    char msg[512];
    char file_path[256];
    int offset = 0;

    if (arg_count < 1) {
        send_string(client_socket, "err-Usage: write [-offset=<num>] <path>");
        return -1;
    }

    // Parse arguments
    if (strncmp(args[0], "-offset=", 8) == 0) {
        if (arg_count < 2) {
            send_string(client_socket, "err-Usage: write [-offset=<num>] <path>");
            return -1;
        }
        offset = atoi(args[0] + 8);
        strncpy(file_path, args[1], sizeof(file_path) - 1);
        file_path[sizeof(file_path) - 1] = '\0';
    } else {
        strncpy(file_path, args[0], sizeof(file_path) - 1);
        file_path[sizeof(file_path) - 1] = '\0';
    }

    // Validate path
    if (check_path(client_socket, id, file_path) != 0) {
        return -1;
    }

    char full_path[PATH_MAX];
    get_full_path(file_path, full_path);
    FileLock *lock = get_file_lock(full_path);
    if (!lock) {
        send_string(client_socket, "err-Server busy (too many locks)");
        return -1;
    }
    writer_lock(lock);

    // Open file (create if not exists, 0700)
    int fd = openat(AT_FDCWD, file_path, O_WRONLY | O_CREAT, 0700);
    if (fd == -1) {
        perror("open failed");
        send_string(client_socket, "err-Error opening/creating file");
        writer_unlock(lock);
        release_file_lock(lock);
        return -1;
    }

    // Seek
    if (offset > 0) {
        if (lseek(fd, offset, SEEK_SET) == -1) {
            perror("lseek failed");
            close(fd);
            send_string(client_socket, "err-Error seeking file");
            writer_unlock(lock);
            release_file_lock(lock);
            return -1;
        }
    }

    send_string(client_socket, "ok-write");

    // Receive content
    char content[4096];
    int bytes_received = recv_all(client_socket, content, sizeof(content) - 1);
    if (bytes_received < 0) {
        close(fd);
        writer_unlock(lock);
        release_file_lock(lock);
        return -1; // recv_all handles error reporting
    }
    content[bytes_received] = '\0';
    
    if (write(fd, content, bytes_received - 1) == -1) {
        perror("write failed");
        close(fd);
        send_string(client_socket, "err-Error writing to file");
        writer_unlock(lock);
        release_file_lock(lock);
        return -1;
    }

    writer_unlock(lock);
    release_file_lock(lock);

    close(fd);
    snprintf(msg, sizeof(msg), "ok-Wrote to %s.", file_path);
    send_string(client_socket, msg);
    return 0;
}

int op_del(int client_socket, int id, DIR *dir, char *args[], int arg_count) {
    (void)dir;
    char msg[256];

    if (arg_count < 1) {
        send_string(client_socket, "err-Usage: delete <path>");
        return -1;
    }

    // Validate path
    if (check_path(client_socket, id, args[0]) != 0) {
        return -1;
    }

    char full_path[PATH_MAX];
    get_full_path(args[0], full_path);
    FileLock *lock = get_file_lock(full_path);
    if (!lock) {
        send_string(client_socket, "err-Server busy (too many locks)");
        return -1;
    }
    writer_lock(lock);

    if (unlink(args[0]) == -1) {
        perror("unlink failed");
        send_string(client_socket, "err-Error deleting file");
        writer_unlock(lock);
        release_file_lock(lock);
        return -1;
    }

    writer_unlock(lock);
    release_file_lock(lock);

    snprintf(msg, sizeof(msg), "ok-Deleted %s.", args[0]);
    send_string(client_socket, msg);
    return 0;
}

void get_full_path(char *path, char *full_path) {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        char temp_path[PATH_MAX+1];
        if (path[0] == '/') {
            snprintf(temp_path, sizeof(temp_path), "%s", path);
        } else {
            snprintf(temp_path, sizeof(temp_path), "%s/%s", cwd, path);
        }
        
        // Try to resolve the full path directly (works if file exists)
        char *real = realpath(temp_path, NULL);
        if (real) {
            strncpy(full_path, real, PATH_MAX - 1);
            full_path[PATH_MAX - 1] = '\0';
            free(real);
        } else {
            // File might not exist, resolve directory
            char *path_dup = strdup(temp_path);
            char *dir = dirname(path_dup);
            char *path_dup2 = strdup(temp_path);
            char *base = basename(path_dup2);
            
            char *real_dir = realpath(dir, NULL);
            if (real_dir) {
                snprintf(full_path, PATH_MAX, "%s/%s", real_dir, base);
                free(real_dir);
            } else {
                // Fallback, just use what we have
                strncpy(full_path, temp_path, PATH_MAX - 1);
                full_path[PATH_MAX - 1] = '\0';
            }
            free(path_dup);
            free(path_dup2);
        }
    }
}

int check_path(int client_socket, int id, char *path) {    
    char *path_copy = strdup(path);
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
        send_string(client_socket, "err-Access denied: Cannot access outside home directory");
        return -1;
    }
    return 0;
    
}