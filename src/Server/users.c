#include "../../include/users.h"
#include "../../include/utils.h"
#include <pwd.h>
//#include "user_session.h"

// this variable should be stored in main
DIR *dir;
// stores the file pointer to the users.dat file
FILE *file;

User users[MAX_USERS];
int user_count = 0;   // number of users in the system
int next_user_id = 0; // next id to be assigned
int user_loaded = 0;  // flag to check if users are loaded

int add_on_local_structure(char *username, mode_t permissions) {
  if (user_count >= MAX_USERS) {
    printf("Maximum number of users reached.\n");
    return -1;
  }
  for (int i = 0; i < user_count; i++) {
    if (strcmp(users[i].username, username) == 0) {
      printf("User %s already exists.\n", username);
      return -1;
    }
  }
  users[user_count].id = next_user_id++;
  strncpy(users[user_count].username, username, USERNAME_LENGTH - 1);
  users[user_count].username[USERNAME_LENGTH - 1] =
      '\0'; // Ensure null-termination
  users[user_count].permissions = permissions;
  user_count++;
  return 0;
}

int delete_from_local_structure(int id) {
  for (int i = 0; i < user_count; i++) {
    if (users[i].id == id) {
      for (int j = i; j < user_count - 1; j++) {
        users[j] = users[j + 1];
      }
      user_count--;
      return 0;
    }
  }
  return -1;
}

int get_id_by_username(char *username) {
  if (!user_loaded) {
    load_users();
  }
  for (int i = 0; i < user_count; i++) {
    if (strcmp(users[i].username, username) == 0) {
      return users[i].id;
    }
  }
  return -1;
}

int get_username_by_id(int id, char *username) {
  if (!user_loaded) {
    load_users();
  }
  for (int i = 0; i < user_count; i++) {
    if (users[i].id == id) {
      strncpy(username, users[i].username, USERNAME_LENGTH);
      return 0;
    }
  }
  return -1;
}

int print_users() {
  if (!user_loaded) {
    load_users();
  }
  printf("User count: %d\n", user_count);
  for (int i = 0; i < user_count; i++) {
    printf("User ID: %d, Name: %s, Permissions: %o\n", users[i].id,
           users[i].username, users[i].permissions);
  }
  return 0;
}

int load_users() {
  file = fopen("users.dat", "rb");
  if (file == NULL) {
    return -1;
  }
  fread(&user_count, sizeof(int), 1, file);
  fread(&next_user_id, sizeof(int), 1, file);
  fread(users, sizeof(User), user_count, file);
  fclose(file);
  printf("Loaded %d users from backup.\n", user_count);
  user_loaded = 1;
  return 0;
}

int save_users() {
  file = fopen("users.dat", "wb");
  if (file == NULL) {
    return -1;
    exit(EXIT_FAILURE);
  }
  fwrite(&user_count, sizeof(int), 1, file);
  fwrite(&next_user_id, sizeof(int), 1, file);
  fwrite(users, sizeof(User), user_count, file);
  fclose(file);
  printf("Saved %d users to backup.\n", user_count);
  return 0;
}

int user_persistent_add(char *username, mode_t permissions) {
  if (!user_loaded) {
    load_users();
  }
  if (add_on_local_structure(username, permissions)) {
    return -1;
  }
  save_users();
  return 0;
}

int user_persistent_delete(int id) {
  if (!user_loaded) {
    load_users();
  }
  if (delete_from_local_structure(id)) {
    return -1;
  }
  save_users();
  return 0;
}

int create_os_user(char *username, mode_t permissions, const char *root_dir) {
  if (!user_loaded) {
    load_users();
  }
  if (get_id_by_username(username) != -1) {
    return -1;
  }

  restore_privileges();
  pid_t pid = fork();
  if (pid == 0) {
    // Child process
    char gid_str[16];
    snprintf(gid_str, sizeof(gid_str), "%d", get_real_uid());
    execlp("useradd", "useradd", "-m", "-g", gid_str, username, NULL);
    perror("execlp useradd failed");
    user_persistent_delete(get_id_by_username(username));
    exit(EXIT_FAILURE);
  } else if (pid > 0) {
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
      if (user_persistent_add(username, permissions)) {
        pid_t pid2 = fork();
        if (pid2 == 0) {
          // Child process
          execlp("userdel", "userdel", "-r", username, NULL);
          perror("execlp userdel failed");
          exit(EXIT_FAILURE);
        } else if (pid2 > 0) {
          int status2;
          waitpid(pid2, &status2, 0);
        }
        return -1;
      }
      char user_dir[512];
      snprintf(user_dir, sizeof(user_dir), "%s/%s", root_dir, username);
      mode_t old_umask = umask(0000); // Set umask to 0, storing the old value
      if (mkdir(user_dir, permissions) == -1) {
        perror("mkdir failed");
        return -1;
      }
      umask(old_umask); // Restore the old umask
      // change the owner of the directory to the user
      struct passwd *pwd = getpwnam(username);
      if (pwd == NULL) {
          perror("getpwnam failed");
      } else {
          if (chown(user_dir, pwd->pw_uid, pwd->pw_gid) == -1) {
              perror("chown failed");
          }
      }
      printf("System user %s created successfully at %s.\n", username, user_dir);
    } else {
      perror("useradd failed");
      user_persistent_delete(get_id_by_username(username));
      return -1;
    }
  }
  minimize_privileges();
  return 0;
}

int delete_os_user(int id, const char *root_dir) {
  if (!user_loaded) {
    load_users();
  }
  char username[USERNAME_LENGTH];
  if (get_username_by_id(id, username)) {
    return -1;
  }

  restore_privileges();
  pid_t pid = fork();
  if (pid == 0) {
    // Child process
    execlp("userdel", "userdel", "-r", username, NULL);
    perror("execlp userdel failed");
    user_persistent_add(username, 0);
    exit(EXIT_FAILURE);
  } else if (pid > 0) {
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
      if (user_persistent_delete(id)) {
        return -1;
      }

      // Delete directory recursively
      pid_t rm_pid = fork();
      if (rm_pid == 0) {
        char user_dir[512];
        snprintf(user_dir, sizeof(user_dir), "%s/%s", root_dir, username);
        execlp("rm", "rm", "-rf", user_dir, NULL);
        perror("execlp rm failed");
        exit(EXIT_FAILURE);
      } else if (rm_pid > 0) {
        waitpid(rm_pid, NULL, 0);
        printf("Directory %s deleted.\n", username);
      } else {
        perror("fork for rm failed");
      }

      printf("System user %s deleted successfully.\n", username);
    } else {
      perror("userdel failed");
      user_persistent_add(username, 0);
      return -1;
    }
  } else {
    perror("userdel failed");
    return -1;
  }
  minimize_privileges();
  return 0;
}

int create_user(char *username, mode_t permissions, const char *root_dir) {
  if (create_os_user(username, permissions, root_dir)) {
    return -1;
  }
  return 0;
}
int delete_user(int id, const char *root_dir) {
  if (delete_os_user(id, root_dir)) {
    return -1;
  }
  return 0;
}