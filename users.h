#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_USERS 25
#define USERNAME_LENGTH 256
#define USERS_FILE "users.dat"

typedef struct {
  int id;
  char username[USERNAME_LENGTH];
  int permissions;
} User;

int create_user(char *username, int permissions);
int delete_user(int id);
int get_id_by_username(char *username);

int save_users();
int load_users();

int print_users();
