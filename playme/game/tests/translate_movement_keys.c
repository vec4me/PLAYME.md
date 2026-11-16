#if 0
fail() {
    printf '%s\n' "$*" >&2
    exit 1
}
cd "$(dirname "$0")/../../.." || fail "could not enter project directory"

mkdir -p build/ || fail "could not create build directory"
${CC:-cc} ${CFLAGS-} \
    "$0" \
    playme/game/state.c \
    playme/numeric.c \
    -o build/translate_movement_keys_test || fail "could not build movement translation test"
build/translate_movement_keys_test || fail "could not translate movement keys"
exit 0
#endif

#include <stdio.h>
#include "../lib.h"

int main(void) {
    if (game_Movement('w') != GAME_MOVEMENT_FORWARD || game_Movement('s') != GAME_MOVEMENT_BACKWARD || game_Movement('a') != GAME_MOVEMENT_TURN_LEFT ||
        game_Movement('d') != GAME_MOVEMENT_TURN_RIGHT || game_Movement('x') != GAME_MOVEMENT_NONE) {
        fprintf(stderr, "movement key translation failed\n");
        return 1;
    }
    return 0;
}
