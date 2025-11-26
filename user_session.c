#include "user_session.h"
#include "users.h"

int user_session(int id) {
  while (1) {
    char command[50];
    char username[USERNAME_LENGTH];
    get_username_by_id(id, username);
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