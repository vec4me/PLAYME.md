#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "internal.h"

static const char *status_text(network_channel_http_status_t status) {
    if (status == NETWORK_CHANNEL_HTTP_OK) {
        return "200 OK";
    }
    if (status == NETWORK_CHANNEL_HTTP_FOUND) {
        return "302 Found";
    }
    return "500 Internal Server Error";
}

static const char *type_text(network_channel_http_media_t media) {
    if (media == NETWORK_CHANNEL_HTTP_JPEG) {
        return "image/jpeg";
    }
    if (media == NETWORK_CHANNEL_HTTP_GIF) {
        return "image/gif";
    }
    return "text/plain";
}

static int send_all(int socket, const void *data, size_t nbytes) {
    const unsigned char *cursor = (const unsigned char *)data;
    ssize_t sent;

    while (0 < nbytes) {
        sent = send(socket, cursor, nbytes, 0);
        if (sent <= 0) {
            return 0;
        }
        cursor += (size_t)sent;
        nbytes -= (size_t)sent;
    }
    return 1;
}

char network_channel_http_receive_action(int socket) {
    char request[1024];
    ssize_t received = recv(socket, request, sizeof(request) - 1, 0);

    if (received <= 0) {
        return '\0';
    }
    request[(size_t)received] = '\0';
    return strncmp(request, "GET /", 5) == 0 ? request[5] : '\0';
}

static int write_response(int socket, const char *status, const char *contenttype, const char *headers, const void *body, size_t nbytes) {
    char response[512];
    int written = sprintf(response, "HTTP/1.1 %s\r\nConnection: close\r\nContent-Type: %s\r\nContent-Length: %lu\r\n%s\r\n", status, contenttype, (unsigned long)nbytes, headers);

    if (written < 0 || sizeof(response) <= (size_t)written || !send_all(socket, response, (size_t)written)) {
        return 0;
    }
    return nbytes == 0 || send_all(socket, body, nbytes);
}

int network_channel_http_respond(int socket, network_channel_http_status_t status, network_channel_http_media_t media, network_channel_http_cache_t cache, const void *body, size_t nbytes) {
    const char *headers = cache == NETWORK_CHANNEL_HTTP_NO_CACHE ? "Cache-Control: max-age=0\r\n" : "";
    return write_response(socket, status_text(status), type_text(media), headers, body, nbytes);
}

int network_channel_http_redirect(int socket, const char *location) {
    char headers[384];
    size_t nlocationchars = 0;
    size_t maxlocationchars = sizeof(headers) - sizeof("Location: \r\n");

    if (location == NULL) {
        return 0;
    }
    while (nlocationchars <= maxlocationchars && location[nlocationchars] != '\0') {
        nlocationchars++;
    }
    if (maxlocationchars < nlocationchars) {
        return 0;
    }
    memcpy(headers, "Location: ", sizeof("Location: ") - 1);
    memcpy(headers + sizeof("Location: ") - 1, location, nlocationchars);
    memcpy(headers + sizeof("Location: ") - 1 + nlocationchars, "\r\n", sizeof("\r\n"));
    return write_response(socket, status_text(NETWORK_CHANNEL_HTTP_FOUND), type_text(NETWORK_CHANNEL_HTTP_PLAIN), headers, NULL, 0);
}
