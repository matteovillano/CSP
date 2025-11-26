#include "users.h"
#include "user_session.h"

// this variable should be stored in main
DIR *dir;
// stores the file pointer to the users.dat file
FILE *file;

User users[MAX_USERS];
int user_count = 0;   // number of users in the system
int next_user_id = 0; // next id to be assigned
int user_loaded = 0;  // flag to check if users are loaded

int add_on_local_structure(char *username, int permissions) {
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
    printf("User ID: %d, Name: %s, Permissions: %d\n", users[i].id,
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

int user_persistent_add(char *username, int permissions) {
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

int create_os_user(char *username, int permissions) {
  if (!user_loaded) {
    load_users();
  }
  if (get_id_by_username(username) != -1) {
    return -1;
  }
  pid_t pid = fork();
  if (pid == 0) {
    // Child process
    execlp("useradd", "useradd", "-m", username, NULL);
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
      if (mkdir(username, 0700) == -1) {
        perror("mkdir failed");
        return -1;
      }
      printf("System user %s created successfully.\n", username);
    } else {
      perror("useradd failed");
      user_persistent_delete(get_id_by_username(username));
      return -1;
    }
  }
  return 0;
}

int delete_os_user(int id) {
  if (!user_loaded) {
    load_users();
  }
  char username[USERNAME_LENGTH];
  if (get_username_by_id(id, username)) {
    return -1;
  }
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
        execlp("rm", "rm", "-rf", username, NULL);
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
  return 0;
}

int create_user(char *username, int permissions) {
  if (create_os_user(username, permissions)) {
    return -1;
  }
  return 0;
}
int delete_user(int id) {
  if (delete_os_user(id)) {
    return -1;
  }
  return 0;
}

/////////////////////////////

int main() {

  char input[300];
  char command[50];
  char username[USERNAME_LENGTH];

  // Ensure users are loaded
  if (!user_loaded) {
    load_users();
  }

  printf("User Management Shell\n");
  printf("Commands: create <username>, delete <username>, login <username>, exit\n");

  while (1) {
    printf("> ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
      break;
    }

    // Remove trailing newline
    input[strcspn(input, "\n")] = 0;

    if (strcmp(input, "exit") == 0) {
      break;
    }

    int num_args = sscanf(input, "%s %s", command, username);

    if (num_args == 2) {
      if (strcmp(command, "create") == 0) {
        create_user(username, 0);
      } else if (strcmp(command, "delete") == 0) {
        int id = get_id_by_username(username);
        if (id != -1) {
          delete_user(id);
        } else {
          printf("User %s not found.\n", username);
        }
      } else if (strcmp(command, "login") == 0) {
        int id=get_id_by_username(username);
        if(id!=-1){
            user_session(id);
        }else{
            printf("User %s not found.\n", username);
        }
      }else {
        printf("Unknown command: %s\n", command);
      }
    } else {
      printf("Invalid command format. Use: create <username> or delete "
             "<username>\n");
    }
    print_users();
  }
  return 0;
}