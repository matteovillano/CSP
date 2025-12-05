#include "../../include/network.h"
#include "../../include/utils.h"
#include "../../include/client_mgmt.h"

int client_socket = -1;
char server_ip[INET_ADDRSTRLEN];
int server_port;

int main(int argc, char *argv[]) {

    strncpy(server_ip, DEFAULT_IP, INET_ADDRSTRLEN);
    server_port = DEFAULT_PORT;

    if (argc > 3) {
        fprintf(stderr, "Too many arguments\n");
        exit(EXIT_FAILURE);
    }

    if (argc >= 2)
        strncpy(server_ip, argv[1], INET_ADDRSTRLEN);
    if (argc >= 3)
        server_port = atoi(argv[2]);

    // connect to server
    client_socket = create_client_socket(server_ip, server_port);
    if (client_socket < 0)
        exit(EXIT_FAILURE);

    printf("Connected to server at %s:%d\n", server_ip, server_port);
    printf("Commands:\n\tcreate_user <username> <permissions>\n\tdelete <username>\n\tlogin <username>\n\texit\n");
    printf("> ");

    char buffer[256];
    // creation and login user
    if (!create_and_login_user(buffer, client_socket)) {
        printf("Exiting\n");
        return 0;
    }
    
    // user session
    if (!client_session(buffer, client_socket)) {
        printf("Exiting\n");
        return 0;
    }

    return 0;
}
