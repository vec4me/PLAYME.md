#if 0
fail() {
    printf '%s\n' "$*" >&2
    exit 1
}
cd "$(dirname "$0")/../" || fail "could not enter project directory"

${CC:-cc} ${CFLAGS-} \
    "$0" \
    build/game_session_c.o \
    build/network_channel_c.o \
    build/network_socket.o \
    build/http.o \
    build/gif_c.o \
    build/game_c.o \
    build/encoded_image.o \
    build/capture_c.o \
    build/jpeg_c.o \
    build/numeric_c.o \
    build/media_c.o \
    build/world.o \
    build/renderer_c.o \
    build/rasterizer_c.o \
    build/text_c.o \
    -o build/preserve_the_game_session_lifetime_test || fail "could not build game session lifetime test"

build/preserve_the_game_session_lifetime_test || fail "could not preserve game session lifetime"
exit 0
#endif

#include <stddef.h>
#include "../server/network_channel/lib.h"

#include "../server/game_session/lib.h"

int main(void) {
    game_session_t session = {{0}};
    network_channel_channel_t channel = {{0}};

    game_session_handle(NULL, &channel);
    game_session_destroy(NULL);
    if (game_session_active(NULL) || game_session_initialize(NULL)) {
        return 1;
    }
    game_session_handle(&session, &channel);
    if (game_session_active(&session) || !game_session_initialize(&session) || !game_session_active(&session)) {
        return 1;
    }
    if (game_session_initialize(&session) || !game_session_active(&session)) {
        return 1;
    }
    game_session_handle(&session, NULL);
    game_session_handle(&session, &channel);
    if (!game_session_active(&session)) {
        return 1;
    }
    game_session_destroy(&session);
    game_session_handle(&session, &channel);
    if (game_session_active(&session)) {
        return 1;
    }
    game_session_destroy(&session);
    if (game_session_active(&session) || !game_session_initialize(&session) || !game_session_active(&session)) {
        return 1;
    }
    game_session_destroy(&session);
    return game_session_active(&session) ? 1 : 0;
}
