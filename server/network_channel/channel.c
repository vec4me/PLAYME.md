#if defined(__STDC_VERSION__) && __STDC_VERSION__ < 199901L
    #include "internal.h"
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ < 202311L
    #include "internal.h"
#elif defined(__STDC_VERSION__)
    #include "internal.h"
#elif defined(__cplusplus)
    #include "internal.h"
#else
    #include "internal.h"
#endif

int network_channel_open(network_channel_listener_t *listener, int port) {
    if (listener == NULL || port < 0 || 65535 < port) {
        return 0;
    }
    return network_channel_socket_open(listener, port);
}

int network_channel_accept(const network_channel_listener_t *listener, network_channel_channel_t *channel) {
    if (listener == NULL || channel == NULL) {
        return 0;
    }
    return network_channel_socket_accept(listener, channel);
}

void network_channel_close_channel(network_channel_channel_t *channel) {
    if (channel != NULL) {
        network_channel_socket_close_channel(channel);
    }
}

void network_channel_close_listener(network_channel_listener_t *listener) {
    if (listener != NULL) {
        network_channel_socket_close_listener(listener);
    }
}

int network_channel_listener_open(const network_channel_listener_t *listener) {
    return listener != NULL && network_channel_socket_listener_open(listener);
}

int network_channel_channel_open(const network_channel_channel_t *channel) {
    return channel != NULL && network_channel_socket_channel_open(channel);
}

char network_channel_receive_action(const network_channel_channel_t *channel) {
    if (!network_channel_channel_open(channel)) {
        return '\0';
    }
    return network_channel_http_receive_action(network_channel_socket_channel_descriptor(channel));
}

int network_channel_respond_image(const network_channel_channel_t *channel, network_channel_image_media_t media, const void *body, size_t nbytes) {
    network_channel_http_media_t type;

    if (body == NULL || nbytes == 0 || !network_channel_channel_open(channel)) {
        return 0;
    }
    if (media == NETWORK_CHANNEL_GIF) {
        type = NETWORK_CHANNEL_HTTP_GIF;
    } else if (media == NETWORK_CHANNEL_JPEG) {
        type = NETWORK_CHANNEL_HTTP_JPEG;
    } else {
        return 0;
    }
    return network_channel_http_respond(network_channel_socket_channel_descriptor(channel), NETWORK_CHANNEL_HTTP_OK, type, NETWORK_CHANNEL_HTTP_NO_CACHE, body, nbytes);
}

int network_channel_respond_failure(const network_channel_channel_t *channel) {
    if (!network_channel_channel_open(channel)) {
        return 0;
    }
    return network_channel_http_respond(network_channel_socket_channel_descriptor(channel), NETWORK_CHANNEL_HTTP_INTERNAL_SERVER_ERROR, NETWORK_CHANNEL_HTTP_PLAIN, NETWORK_CHANNEL_HTTP_DEFAULT, NULL, 0);
}

int network_channel_redirect(const network_channel_channel_t *channel, const char *location) {
    if (location == NULL || !network_channel_channel_open(channel)) {
        return 0;
    }
    return network_channel_http_redirect(network_channel_socket_channel_descriptor(channel), location);
}
