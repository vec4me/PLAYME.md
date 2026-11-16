#if 0
fail() {
    printf '%s\n' "$1" >&2
    exit 1
}

cd "$(dirname "$0")/../../.." || fail "could not enter project directory"
mkdir -p build/ || fail "could not create build directory"
${CC:-cc} ${CFLAGS-} \
    "$0" \
    playme/game/state.c \
    playme/numeric.c \
    -o build/move_forward_at_the_game_speed_test || fail "could not build movement speed test"
build/move_forward_at_the_game_speed_test || fail "movement did not use the game speed"
exit 0
#endif

#include "../../numeric.h"
#include "../lib.h"

int main(void) {
    game_t game = {{0}};
    game_frame_t before;
    game_frame_t after;

    if (!game_initialize(&game)) {
        return 1;
    }
    if (!game_observe_frame(&game, &before)) {
        return 1;
    }
    game_apply_movement(&game, GAME_MOVEMENT_FORWARD);
    if (!game_observe_frame(&game, &after)) {
        game_destroy(&game);
        return 1;
    }
    if (numeric_RawValue(after.camera.pos.x) != numeric_RawValue(before.camera.pos.x) || numeric_RawValue(after.camera.pos.y) != numeric_RawValue(before.camera.pos.y) ||
        numeric_RawValue(after.camera.pos.z) != numeric_RawValue(before.camera.pos.z) - numeric_RawValue(numeric_FixedFromInteger(13)) || after.nmoves != 1) {
        game_destroy(&game);
        return 1;
    }
    game_destroy(&game);
    return 0;
}
