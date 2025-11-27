#ifndef OPS_H
#define OPS_H

#include <dirent.h>

int op_create(int id, DIR* dir, char* args[], int arg_count);
int op_changemod(int id, DIR* dir, char* args[], int arg_count);
int op_move(int id, DIR* dir, char* args[], int arg_count);
int op_upload(int id, DIR* dir, char* args[], int arg_count);
int op_download(int id, DIR* dir, char* args[], int arg_count);
int op_cd(int id, DIR* dir, char* args[], int arg_count);
int op_list(int id, DIR* dir, char* args[], int arg_count);
int op_read(int id, DIR* dir, char* args[], int arg_count);
int op_write(int id, DIR* dir, char* args[], int arg_count);
int op_del(int id, DIR* dir, char* args[], int arg_count);

#endif
