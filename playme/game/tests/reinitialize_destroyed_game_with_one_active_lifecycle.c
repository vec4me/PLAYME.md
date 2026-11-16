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
    -o build/reinitialize_destroyed_game_with_one_active_lifecycle_test || fail "could not build game lifecycle test"
build/reinitialize_destroyed_game_with_one_active_lifecycle_test || fail "could not reinitialize the destroyed game"
exit 0
#endif

#include "../lib.h"

int main(void) {
    game_t game = {{0}};
    game_frame_t frame;

    if (!game_initialize(&game) || game_initialize(&game)) {
        return 1;
    }
    game_destroy(&game);
    game_destroy(&game);
    if (game_observe_frame(&game, &frame) || !game_initialize(&game) || !game_observe_frame(&game, &frame) || game_initialize(&game)) {
        game_destroy(&game);
        return 1;
    }
    game_destroy(&game);
    return 0;
}
