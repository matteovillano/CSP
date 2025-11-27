#include "ops.h"
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int op_create(int id, DIR *dir, char *args[], int arg_count) {
  if (arg_count < 1) {
    printf("Usage: create <filename>\n");
    return -1;
  }

  int fd = openat(dirfd(dir), args[0], O_CREAT | O_WRONLY | O_EXCL, 0644);
  if (fd == -1) {
    perror("Error creating file");
    return -1;
  }

  close(fd);
  printf("File %s created successfully.\n", args[0]);
  return 0;
}

int op_changemod(int id, DIR *dir, char *args[], int arg_count) {
  printf("changemod\n");
  return 0;
}

int op_move(int id, DIR *dir, char *args[], int arg_count) {
  printf("move\n");
  return 0;
}

int op_upload(int id, DIR *dir, char *args[], int arg_count) {
  printf("upload\n");
  return 0;
}

int op_download(int id, DIR *dir, char *args[], int arg_count) {
  printf("download\n");
  return 0;
}

int op_cd(int id, DIR *dir, char *args[], int arg_count) {
  printf("cd\n");
  return 0;
}

int op_list(int id, DIR *dir, char *args[], int arg_count) {
  printf("list\n");
  return 0;
}

int op_read(int id, DIR *dir, char *args[], int arg_count) {
  printf("read\n");
  return 0;
}

int op_write(int id, DIR *dir, char *args[], int arg_count) {
  printf("write\n");
  return 0;
}
int op_del(int id, DIR *dir, char *args[], int arg_count) {
  printf("del\n");
  return 0;
}