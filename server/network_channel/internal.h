#ifndef SERVER_NETWORK_CHANNEL_INTERNAL_H
#define SERVER_NETWORK_CHANNEL_INTERNAL_H

#include "lib.h"

typedef enum {
    NETWORK_CHANNEL_HTTP_OK,
    NETWORK_CHANNEL_HTTP_FOUND,
    NETWORK_CHANNEL_HTTP_INTERNAL_SERVER_ERROR
} network_channel_http_status_t;

typedef enum {
    NETWORK_CHANNEL_HTTP_PLAIN,
    NETWORK_CHANNEL_HTTP_JPEG,
    NETWORK_CHANNEL_HTTP_GIF
} network_channel_http_media_t;

typedef enum {
    NETWORK_CHANNEL_HTTP_DEFAULT,
    NETWORK_CHANNEL_HTTP_NO_CACHE
} network_channel_http_cache_t;

int network_channel_socket_open(network_channel_listener_t *listener, int port);
int network_channel_socket_accept(const network_channel_listener_t *listener, network_channel_channel_t *channel);
void network_channel_socket_close_channel(network_channel_channel_t *channel);
void network_channel_socket_close_listener(network_channel_listener_t *listener);
int network_channel_socket_listener_open(const network_channel_listener_t *listener);
int network_channel_socket_channel_open(const network_channel_channel_t *channel);
int network_channel_socket_channel_descriptor(const network_channel_channel_t *channel);

char network_channel_http_receive_action(int socket);
int network_channel_http_respond(int socket, network_channel_http_status_t status, network_channel_http_media_t media, network_channel_http_cache_t cache, const void *body, size_t nbytes);
int network_channel_http_redirect(int socket, const char *location);

#endif
