#include "../../include/network.h"
#include <dirent.h> // For opendir and closedir
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/stat.h> // For stat and mkdir
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[]) {



  printf("Hello World!!!\n");
  char *root_dir;
  char *ip = DEFAULT_IP;
  int port = DEFAULT_PORT;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <root_dir> [ip] [port]\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  char cwd[1024];
  if (getcwd(cwd, sizeof(cwd)) == NULL) {
    perror("getcwd");
    exit(EXIT_FAILURE);
  }

  char *arg_path = argv[1];
  while (*arg_path == '/') {
    arg_path++;
  }

  root_dir = malloc(strlen(cwd) + 1 + strlen(arg_path) + 1);
  if (!root_dir) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  sprintf(root_dir, "%s/%s", cwd, arg_path);

  if (argc > 2) {
    ip = argv[2];
  }
  if (argc > 3) {
    port = atoi(argv[3]);
  }

  struct stat st = {0};
  if (stat(root_dir, &st) == -1) {
    if (mkdir(root_dir, 0777) == -1) {
      perror("mkdir");
      exit(EXIT_FAILURE);
    }
    printf("Created root directory: %s\n", root_dir);
  }

  DIR *dir = opendir(root_dir);
  if (dir) {
    printf("Opened root directory: %s\n", root_dir);
    closedir(dir);
  } else {
    perror("opendir");
    exit(EXIT_FAILURE);
  }

  printf("ip: %s\n", ip);
  printf("port: %d\n", port);
  printf("root_dir: %s\n", root_dir);

  printf("\n");

  return 0;
}
