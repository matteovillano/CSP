#ifndef USER_SESSION_H
#define USER_SESSION_H

#define MAX_COMMAND_LENGTH 256

int user_session(int client_socket, int id);
int handle_client(int client_socket, const char *root_dir);

#endif