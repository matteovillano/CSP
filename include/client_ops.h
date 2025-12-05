#ifndef CLIENT_OPS_H
#define CLIENT_OPS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void upload_client(char* buffer, int client_socket);
void read_client(char* buffer, int client_socket);
void download_client(char* buffer, int client_socket);

#endif