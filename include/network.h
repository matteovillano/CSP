#ifndef NETWORK_H
#define NETWORK_H

#include "utils.h"

#define DEFAULT_PORT 8080
#define DEFAULT_IP "127.0.0.1"

// Create a server socket bound to the specified port
// Returns the socket file descriptor or -1 on error
int create_server_socket(int port);

// Create a client socket connected to the specified IP and port
// Returns the socket file descriptor or -1 on error
int create_client_socket(const char *ip, int port);

#endif // NETWORK_H
