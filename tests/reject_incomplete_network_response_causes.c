#if 0
fail() {
    printf '%s\n' "$*" >&2
    exit 1
}
cd "$(dirname "$0")/../" || fail "could not enter project directory"

mkdir -p build/ || fail "could not create build directory"
${CC:-cc} ${CFLAGS-} \
    "$0" \
    server/network_channel/channel.c \
    server/network_channel/socket.c \
    server/network_channel/http.c \
    -o build/reject_incomplete_network_response_causes_test || fail "could not build incomplete network response cause test"

build/reject_incomplete_network_response_causes_test || fail "could not reject incomplete network response causes"
exit 0
#endif

#include <stddef.h>
#include "../server/network_channel/lib.h"

int main(void) {
    network_channel_listener_t listener = {{0}};
    network_channel_channel_t channel = {{0}};
    unsigned char body = 0;

    network_channel_close_channel(NULL);
    network_channel_close_listener(NULL);
    if (network_channel_open(NULL, 0) || network_channel_open(&listener, -1) || network_channel_open(&listener, 65536) || network_channel_accept(NULL, &channel) ||
        network_channel_accept(&listener, NULL) || network_channel_listener_open(NULL) || network_channel_listener_open(&listener) || network_channel_channel_open(NULL) ||
        network_channel_receive_action(NULL) != '\0' || network_channel_respond_failure(NULL) || network_channel_respond_image(NULL, NETWORK_CHANNEL_JPEG, &body, 1) ||
        network_channel_respond_image(&channel, NETWORK_CHANNEL_JPEG, NULL, 1) || network_channel_respond_image(&channel, NETWORK_CHANNEL_JPEG, &body, 0) ||
        network_channel_respond_image(&channel, (network_channel_image_media_t)99, &body, 1) || network_channel_redirect(&channel, NULL) ||
        network_channel_redirect(NULL, "https://example.com") || network_channel_channel_open(&channel)) {
        return 1;
    }
    return 0;
}
