#if 0
fail() {
    printf '%s\n' "$*" >&2
    exit 1
}
cd "$(dirname "$0")/../../.." || fail "could not enter project directory"

sh tools/build_server.sh || fail "could not build Playme library"
${CC:-cc} ${CFLAGS-} "$0" build/gif_c.o build/game_c.o build/encoded_image.o build/capture_c.o build/jpeg_c.o build/media_c.o build/world.o build/numeric_c.o build/renderer_c.o build/rasterizer_c.o build/text_c.o ${LDFLAGS-} ${LDLIBS-} -o build/preserve_unit_rotation_while_turning_test || fail "could not build unit rotation test"
build/preserve_unit_rotation_while_turning_test || fail "could not preserve unit rotation while turning"
exit 0
#endif

#include <stdio.h>
#include "../../numeric.h"
#include "../lib.h"

int main(void) {
    game_t game = {{0}};
    game_frame_t frame;
    long norm;
    int turn;

    if (!game_initialize(&game)) {
        return 1;
    }
    for (turn = 0; turn < 256; turn++) {
        game_apply_movement(&game, GAME_MOVEMENT_TURN_LEFT);
    }
    if (!game_observe_frame(&game, &frame)) {
        return 1;
    }
    norm = (long)numeric_RawValue(frame.camera.rot.x) * numeric_RawValue(frame.camera.rot.x) + (long)numeric_RawValue(frame.camera.rot.y) * numeric_RawValue(frame.camera.rot.y) +
           (long)numeric_RawValue(frame.camera.rot.z) * numeric_RawValue(frame.camera.rot.z) + (long)numeric_RawValue(frame.camera.rot.w) * numeric_RawValue(frame.camera.rot.w);
    if (norm < 1024L * 1024L - 2048 || 1024L * 1024L + 2048 < norm) {
        fprintf(stderr, "turning changed rotation norm to %ld\n", norm);
        return 1;
    }
    game_destroy(&game);
    return 0;
}
