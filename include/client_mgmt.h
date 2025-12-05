#ifndef CLIENT_SESSION_H
#define CLIENT_SESSION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int create_and_login_user(char* buffer, int client_socket);
int client_session(char* buffer, int client_socket);

#endif