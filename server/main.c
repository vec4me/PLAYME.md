#include <stdio.h>
#include "runtime.h"

static int Port(const char *text) {
    int port = 0;
    int digit;

    if (*text == '\0') {
        return 0;
    }
    while (*text != '\0') {
        if (*text < '0' || '9' < *text) {
            return 0;
        }
        digit = *text - '0';
        if ((65535 - digit) / 10 < port) {
            return 0;
        }
        port = port * 10 + digit;
        text++;
    }
    return port;
}

int main(int nargs, char **args) {
    int port;

    if (nargs != 2) {
        fprintf(stderr, "usage: playme_server <port>\n");
        return 1;
    }
    port = Port(args[1]);
    if (port == 0) {
        fprintf(stderr, "invalid port: %s\n", args[1]);
        return 1;
    }
    return playme_server_run(port);
}
