#ifndef SERVER_NETWORK_CHANNEL_LIB_H
#define SERVER_NETWORK_CHANNEL_LIB_H

#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef union {
    unsigned char storage[sizeof(unsigned)];
    unsigned align;
} network_channel_listener_t;

typedef union {
    unsigned char storage[sizeof(unsigned)];
    unsigned align;
} network_channel_channel_t;

typedef enum {
    NETWORK_CHANNEL_GIF,
    NETWORK_CHANNEL_JPEG
} network_channel_image_media_t;

int network_channel_open(network_channel_listener_t *listener, int port);
int network_channel_accept(const network_channel_listener_t *listener, network_channel_channel_t *channel);
void network_channel_close_channel(network_channel_channel_t *channel);
void network_channel_close_listener(network_channel_listener_t *listener);
int network_channel_listener_open(const network_channel_listener_t *listener);
int network_channel_channel_open(const network_channel_channel_t *channel);
char network_channel_receive_action(const network_channel_channel_t *channel);
int network_channel_respond_image(const network_channel_channel_t *channel, network_channel_image_media_t media, const void *body, size_t nbytes);
int network_channel_respond_failure(const network_channel_channel_t *channel);
int network_channel_redirect(const network_channel_channel_t *channel, const char *location);

#if defined(__cplusplus)
}
#endif

#endif
