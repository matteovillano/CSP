#include "../../include/network.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    printf("Hello World!!!\n");
    char *ip = DEFAULT_IP;
    int port = DEFAULT_PORT;

    if (argc > 2) {
        ip = argv[2];
    }
    if (argc > 3) {
        port = atoi(argv[3]);
    }

    

    return 0;
} 