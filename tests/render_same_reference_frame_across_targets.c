#if 0
cd "$(dirname "$0")/.." || exit 1
sh tools/build_server.sh || exit 1
${CC:-cc} ${CFLAGS-} "$0" build/gif_c.o build/game_c.o build/encoded_image.o build/capture_c.o build/jpeg_c.o build/media_c.o build/world.o build/numeric_c.o build/renderer_c.o build/rasterizer_c.o build/text_c.o ${LDFLAGS-} ${LDLIBS-} -o build/render_same_reference_frame_across_targets_test || exit 1
build/render_same_reference_frame_across_targets_test >/dev/null
exit $?
#endif

#include <stdio.h>

#include "../playme/game/lib.h"
#include "../playme/numeric.h"
#include "../playme/renderer/lib.h"

int main(void) {
    game_t game = {{0}};
    game_frame_t observation;
    renderer_framebuffer_t frame = {{0}};
    renderer_raycache_t rays = {{0}};
    numeric_vector_t size = {320, 200, 0, 0};
    size_t nbytes;
    size_t nwrittenbytes;
    int movement;
    int status = 1;

    if (!game_initialize(&game)) {
        return 1;
    }
    game_set_time(&game, 72);
    nbytes = (size_t)size.x * size.y * 3;
    for (movement = 0; movement < 32; movement++) {
        game_apply_movement(&game, GAME_MOVEMENT_FORWARD);
        game_apply_movement(&game, GAME_MOVEMENT_TURN_LEFT);
        if (!game_observe_frame(&game, &observation)) {
            goto cleanup;
        }
        if (!renderer_create_cached(&rays, &observation.camera.pos, &observation.camera.rot, &observation.sun, &size, &frame)) {
            goto cleanup;
        }
        nwrittenbytes = fwrite(renderer_Pixels(&frame), 1, nbytes, stdout);
        if (nwrittenbytes != nbytes) {
            goto cleanup;
        }
    }
    status = 0;

cleanup:
    renderer_destroy_framebuffer(&frame);
    renderer_destroy_cache(&rays);
    game_destroy(&game);
    return status;
}
