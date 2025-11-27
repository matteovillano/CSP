#ifndef OPS_H
#define OPS_H

#include <dirent.h>

int create(int id, DIR* dir, char* command);
int chmod(int id, DIR* dir, char* command);
int move(int id, DIR* dir, char* command);
int upload(int id, DIR* dir, char* command);
int download(int id, DIR* dir, char* command);
int cd(int id, DIR* dir, char* command);
int list(int id, DIR* dir, char* command);
int read(int id, DIR* dir, char* command);
int write(int id, DIR* dir, char* command);
int del(int id, DIR* dir, char* command);

#endif
