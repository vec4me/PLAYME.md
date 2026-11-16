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
    -o build/preserve_network_resource_lifetimes_test || fail "could not build network resource lifetime test"

build/preserve_network_resource_lifetimes_test || fail "could not preserve network resource lifetimes"
exit 0
#endif

#include "../server/network_channel/lib.h"

int main(void) {
    network_channel_listener_t listener = {{0}};
    network_channel_channel_t channel = {{0}};
    const network_channel_listener_t *observed = &listener;

    if (network_channel_listener_open(&listener) || network_channel_channel_open(&channel) || network_channel_accept(observed, &channel) || network_channel_channel_open(&channel)) {
        return 1;
    }
    if (!network_channel_open(&listener, 0) || !network_channel_listener_open(&listener)) {
        return 1;
    }
    if (network_channel_open(&listener, 0) || !network_channel_listener_open(&listener)) {
        return 1;
    }
    network_channel_close_listener(&listener);
    if (network_channel_listener_open(&listener)) {
        return 1;
    }
    network_channel_close_listener(&listener);
    network_channel_close_channel(&channel);
    return network_channel_listener_open(&listener) || network_channel_channel_open(&channel) ? 1 : 0;
}
