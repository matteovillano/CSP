
#include "user_session.h"
#include "users.h"

int main() {

  char input[MAX_COMMAND_LENGTH];
  char command[MAX_COMMAND_LENGTH];
  char username[USERNAME_LENGTH];

  printf("User Management Shell\n");
  printf("Commands: create <username>, delete <username>, login <username>, "
         "exit\n");

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
        int id = get_id_by_username(username);
        if (id != -1) {
          user_session(id);
          return 0;
        } else {
          printf("User %s not found.\n", username);
        }
      } else {
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