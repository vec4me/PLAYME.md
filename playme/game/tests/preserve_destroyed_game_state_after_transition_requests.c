#if 0
fail() {
    printf '%s\n' "$*" >&2
    exit 1
}
cd "$(dirname "$0")/../../.." || fail "could not enter project directory"

sh tools/build_server.sh || fail "could not build Playme library"
${CC:-cc} ${CFLAGS-} "$0" build/gif_c.o build/game_c.o build/encoded_image.o build/capture_c.o build/jpeg_c.o build/media_c.o build/world.o build/numeric_c.o build/renderer_c.o build/rasterizer_c.o build/text_c.o ${LDFLAGS-} ${LDLIBS-} -o build/preserve_destroyed_game_state_after_transition_requests_test || fail "could not build destroyed game state test"
build/preserve_destroyed_game_state_after_transition_requests_test || fail "could not preserve destroyed game state after transition requests"
exit 0
#endif

#include <string.h>
#include "../../numeric.h"
#include "../lib.h"

int main(void) {
    game_t game = {{0}};
    game_t destroyed;
    game_frame_t frame;

    game_destroy(NULL);
    game_set_time(NULL, 64);
    game_set_current_time(NULL);
    game_apply_movement(NULL, GAME_MOVEMENT_FORWARD);
    game_move_player(NULL, numeric_FixedFromInteger(1), numeric_FixedFromInteger(1));
    game_set_view_rotation(NULL, 32, 16);
    if (game_initialize(NULL) || game_observe_frame(NULL, &frame)) {
        return 1;
    }
    if (!game_initialize(&game)) {
        return 1;
    }
    if (game_observe_frame(&game, NULL)) {
        return 1;
    }
    game_destroy(&game);
    destroyed = game;

    game_set_time(&game, 64);
    game_set_current_time(&game);
    game_apply_movement(&game, GAME_MOVEMENT_FORWARD);
    game_move_player(&game, numeric_FixedFromInteger(1), numeric_FixedFromInteger(1));
    game_set_view_rotation(&game, 32, 16);

    return memcmp(&game, &destroyed, sizeof(game)) != 0 || game_observe_frame(&game, &frame) ? 1 : 0;
}
