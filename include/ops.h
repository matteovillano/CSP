#ifndef OPS_H
#define OPS_H

#include <dirent.h>

int op_create(int client_socket, int id, DIR* dir, char* args[], int arg_count);
int op_changemod(int client_socket, int id, DIR* dir, char* args[], int arg_count);
int op_move(int client_socket, int id, DIR* dir, char* args[], int arg_count);
int op_upload(int client_socket, int id, DIR* dir, char* args[], int arg_count);
int op_download(int client_socket, int id, DIR* dir, char* args[], int arg_count);
int op_cd(int client_socket, int id, DIR* dir, char* args[], int arg_count);
int op_list(int client_socket, int id, DIR* dir, char* args[], int arg_count);
int op_read(int client_socket, int id, DIR* dir, char* args[], int arg_count);
int op_write(int client_socket, int id, DIR* dir, char* args[], int arg_count);
int op_del(int client_socket, int id, DIR* dir, char* args[], int arg_count);

#endif