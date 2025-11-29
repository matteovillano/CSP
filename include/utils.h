#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Helper to print errors
void print_error(const char *msg);

// Helper to send all data
int send_all(int socket, char *buffer, int length);

// Helper to receive all data
int recv_all(int socket, char *buffer, int length);

// Helper to send a string
void send_string(int client_socket, char *str);

// Privilege management
void init_privileges();
void minimize_privileges();
void restore_privileges();
uid_t get_real_uid();

#endif // UTILS_H