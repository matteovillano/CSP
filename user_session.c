#include "user_session.h"
#include "users.h"

int user_session(int id) {
  char username[USERNAME_LENGTH];
  get_username_by_id(id, username);

  DIR *dir;
  dir = opendir(username);
  if (dir == NULL) {
    printf("Error opening directory\n");
    return -1;
  }
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    printf("%s\n", entry->d_name);
  }
  closedir(dir);
  while (1) {
    char command[MAX_COMMAND_LENGTH];

    printf("User %s> ", username);
    scanf("%s", command);

    if (strcmp(command, "exit") == 0) {
      break;
    }
    if (strcmp(command, "create") == 0) {
      printf("I execute create!\n");
    }
    if (strcmp(command, "chmod") == 0) {
      printf("I execute chmod!\n");
    }
    if (strcmp(command, "move") == 0) {
      printf("I execute move!\n");
    }
    if (strcmp(command, "upload") == 0) {
      printf("I execute upload!\n");
    }
    if (strcmp(command, "download") == 0) {
      printf("I execute download!\n");
    }
    if (strcmp(command, "cd") == 0) {
      printf("I execute rename!\n");
    }
    if (strcmp(command, "list") == 0) {
      printf("I execute list!\n");
    }
    if (strcmp(command, "read") == 0) {
      printf("I execute read!\n");
    }
    if (strcmp(command, "write") == 0) {
      printf("I execute write!\n");
    }
    if (strcmp(command, "delete") == 0) {
      printf("I execute delete!\n");
    }
  }
  return 0;
}