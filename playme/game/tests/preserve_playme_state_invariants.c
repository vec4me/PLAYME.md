#if 0
fail() {
    printf '%s\n' "$*" >&2
    exit 1
}
cd "$(dirname "$0")/../../.." || fail "could not enter project directory"

sh tools/build_server.sh || fail "could not build Playme library"
${CC:-cc} ${CFLAGS-} "$0" build/gif_c.o build/game_c.o build/encoded_image.o build/capture_c.o build/jpeg_c.o build/media_c.o build/world.o build/numeric_c.o build/renderer_c.o build/rasterizer_c.o build/text_c.o ${LDFLAGS-} ${LDLIBS-} -o build/preserve_playme_state_invariants_test || fail "could not build Playme state invariant test"
build/preserve_playme_state_invariants_test || fail "could not preserve Playme state invariants"
exit 0
#endif

#include <stdio.h>
#include "../../numeric.h"
#include "../lib.h"

static int VectorsEqual(numeric_fixedvector_t a, numeric_fixedvector_t b) {
    return numeric_RawValue(a.x) == numeric_RawValue(b.x) && numeric_RawValue(a.y) == numeric_RawValue(b.y) && numeric_RawValue(a.z) == numeric_RawValue(b.z) && numeric_RawValue(a.w) == numeric_RawValue(b.w);
}

static int RotationsEqual(numeric_quaternion_t a, numeric_quaternion_t b) {
    return numeric_RawValue(a.x) == numeric_RawValue(b.x) && numeric_RawValue(a.y) == numeric_RawValue(b.y) && numeric_RawValue(a.z) == numeric_RawValue(b.z) && numeric_RawValue(a.w) == numeric_RawValue(b.w);
}

int main(void) {
    game_t game = {{0}};
    game_frame_t frame;
    game_frame_t before;
    numeric_quaternion_t viewrot;

    if (!game_initialize(&game)) {
        return 1;
    }
    if (!game_observe_frame(&game, &before) || game_initialize(&game) || !game_observe_frame(&game, &frame) || !VectorsEqual(frame.camera.pos, before.camera.pos) ||
        !RotationsEqual(frame.camera.rot, before.camera.rot) || frame.nmoves != before.nmoves) {
        fprintf(stderr, "repeated initialization changed game state\n");
        game_destroy(&game);
        return 1;
    }
    game_set_view_rotation(&game, 32, 16);
    if (!game_observe_frame(&game, &before)) {
        return 1;
    }
    viewrot = before.camera.rot;
    game_move_player(&game, numeric_FixedFromInteger(1), numeric_FixedFromInteger(0));
    if (!game_observe_frame(&game, &frame) || !RotationsEqual(frame.camera.rot, viewrot)) {
        fprintf(stderr, "movement did not preserve the view invariant\n");
        return 1;
    }
    before = frame;
    game_apply_movement(&game, GAME_MOVEMENT_TURN_LEFT);
    if (!game_observe_frame(&game, &frame) || !VectorsEqual(frame.camera.pos, before.camera.pos) || frame.nmoves != before.nmoves + 1) {
        fprintf(stderr, "discrete input did not preserve the view invariant\n");
        return 1;
    }
    game_set_view_rotation(&game, 16, 8);
    if (!game_observe_frame(&game, &before)) {
        return 1;
    }
    game_apply_movement(&game, (game_movement_t)99);
    if (!game_observe_frame(&game, &frame) || frame.nmoves != before.nmoves || !VectorsEqual(frame.camera.pos, before.camera.pos) || !RotationsEqual(frame.camera.rot, before.camera.rot) ||
        !VectorsEqual(frame.sun, before.sun)) {
        fprintf(stderr, "invalid input changed game state\n");
        return 1;
    }
    game_destroy(&game);
    return 0;
}
