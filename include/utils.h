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
int send_all(int socket, const void *buffer, size_t length);

// Helper to receive all data
int recv_all(int socket, void *buffer, size_t length);

#endif // UTILS_H