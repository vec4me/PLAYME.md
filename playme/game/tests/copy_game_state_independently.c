#if 0
fail() {
    printf '%s\n' "$*" >&2
    exit 1
}
cd "$(dirname "$0")/../../.." || fail "could not enter project directory"

sh tools/build_server.sh || fail "could not build Playme library"
${CC:-cc} ${CFLAGS-} "$0" build/gif_c.o build/game_c.o build/encoded_image.o build/capture_c.o build/jpeg_c.o build/media_c.o build/world.o build/numeric_c.o build/renderer_c.o build/rasterizer_c.o build/text_c.o ${LDFLAGS-} ${LDLIBS-} -o build/copy_game_state_independently_test || fail "could not build copy game state independently test"
build/copy_game_state_independently_test || fail "could not copy game state independently"
exit 0
#endif

#include <stdio.h>
#include "../../numeric.h"
#include "../lib.h"

static int VectorsEqual(numeric_fixedvector_t a, numeric_fixedvector_t b) {
    return numeric_RawValue(a.x) == numeric_RawValue(b.x) && numeric_RawValue(a.y) == numeric_RawValue(b.y) && numeric_RawValue(a.z) == numeric_RawValue(b.z) && numeric_RawValue(a.w) == numeric_RawValue(b.w);
}

int main(void) {
    game_t game = {{0}};
    game_t copy;
    game_frame_t copyframe;
    game_frame_t observedcopy;
    game_frame_t gameframe;

    if (!game_initialize(&game)) {
        return 1;
    }
    copy = game;
    if (!game_observe_frame(&copy, &copyframe)) {
        return 1;
    }
    game_move_player(&game, numeric_FixedFromInteger(1), numeric_FixedFromInteger(0));
    if (!game_observe_frame(&copy, &observedcopy) || !game_observe_frame(&game, &gameframe) || !VectorsEqual(observedcopy.camera.pos, copyframe.camera.pos) ||
        VectorsEqual(gameframe.camera.pos, copyframe.camera.pos)) {
        fprintf(stderr, "copied game state shared mutation authority\n");
        return 1;
    }
    game_destroy(&game);
    game_destroy(&copy);
    return 0;
}
