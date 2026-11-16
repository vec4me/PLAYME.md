#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "internal.h"

typedef union {
    struct sockaddr socket;
    struct sockaddr_in internet;
} network_channel_address_t;

static unsigned *listener_handle(network_channel_listener_t *listener) {
    return (unsigned *)listener->storage;
}

static const unsigned *constant_listener_handle(const network_channel_listener_t *listener) {
    return (const unsigned *)listener->storage;
}

static unsigned *channel_handle(network_channel_channel_t *channel) {
    return (unsigned *)channel->storage;
}

static const unsigned *constant_channel_handle(const network_channel_channel_t *channel) {
    return (const unsigned *)channel->storage;
}

static int listener_descriptor(const network_channel_listener_t *listener) {
    unsigned handle = *constant_listener_handle(listener);
    return handle == 0 ? -1 : (int)(handle - 1);
}

int network_channel_socket_channel_descriptor(const network_channel_channel_t *channel) {
    unsigned handle = *constant_channel_handle(channel);
    return handle == 0 ? -1 : (int)(handle - 1);
}

static void set_listener_descriptor(network_channel_listener_t *listener, int descriptor) {
    *listener_handle(listener) = descriptor < 0 ? 0U : (unsigned)descriptor + 1U;
}

static void set_channel_descriptor(network_channel_channel_t *channel, int descriptor) {
    *channel_handle(channel) = descriptor < 0 ? 0U : (unsigned)descriptor + 1U;
}

int network_channel_socket_listener_open(const network_channel_listener_t *listener) {
    return 0 <= listener_descriptor(listener);
}

int network_channel_socket_channel_open(const network_channel_channel_t *channel) {
    return 0 <= network_channel_socket_channel_descriptor(channel);
}

int network_channel_socket_open(network_channel_listener_t *listener, int port) {
    network_channel_address_t address;
    int descriptor;

    if (network_channel_socket_listener_open(listener)) {
        return 0;
    }
    memset(&address, 0, sizeof(address));
    descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (descriptor < 0) {
        perror("socket");
        return 0;
    }
    address.internet.sin_family = AF_INET;
    address.internet.sin_port = htons((unsigned short)port);
    address.internet.sin_addr.s_addr = INADDR_ANY;
    if (bind(descriptor, &address.socket, sizeof(address.internet)) < 0 || listen(descriptor, 5) < 0) {
        close(descriptor);
        return 0;
    }
    set_listener_descriptor(listener, descriptor);
    return 1;
}

int network_channel_socket_accept(const network_channel_listener_t *listener, network_channel_channel_t *channel) {
    int descriptor;

    if (!network_channel_socket_listener_open(listener) || network_channel_socket_channel_open(channel)) {
        return 0;
    }
    descriptor = accept(listener_descriptor(listener), NULL, NULL);
    if (descriptor < 0) {
        return 0;
    }
    set_channel_descriptor(channel, descriptor);
    return 1;
}

void network_channel_socket_close_channel(network_channel_channel_t *channel) {
    int descriptor = network_channel_socket_channel_descriptor(channel);
    if (0 <= descriptor) {
        close(descriptor);
        set_channel_descriptor(channel, -1);
    }
}

void network_channel_socket_close_listener(network_channel_listener_t *listener) {
    int descriptor = listener_descriptor(listener);
    if (0 <= descriptor) {
        close(descriptor);
        set_listener_descriptor(listener, -1);
    }
}
