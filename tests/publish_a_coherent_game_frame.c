#if 0
fail() {
    printf '%s\n' "$*" >&2
    exit 1
}
cd "$(dirname "$0")/../" || fail "could not enter project directory"

mkdir -p build/ || fail "could not create build directory"
${CC:-cc} ${CFLAGS-} \
    "$0" \
    playme/capture/frame.c \
    playme/game/state.c \
    playme/media/encoded_image.c \
    playme/media/gif.c \
    playme/media/jpeg.c \
    playme/media/encoding.c \
    playme/numeric.c \
    playme/renderer/frame.c \
    playme/renderer/rasterizer.c \
    playme/renderer/text.c \
    playme/renderer/world.c \
    -o build/publish_a_coherent_game_frame_test || fail "could not build capture invariant test"
build/publish_a_coherent_game_frame_test || fail "could not publish a coherent game frame"
exit 0
#endif

#include <stddef.h>
#include <stdio.h>
#include "../playme/numeric.h"
#include "../playme/game/lib.h"
#include "../playme/renderer/lib.h"
#include "../playme/capture/lib.h"


static int frame_valid(const renderer_framebuffer_t *frame, const numeric_vector_t *size) {
    numeric_vector_t published = renderer_FramebufferSize(frame);

    return renderer_Pixels(frame) != NULL && renderer_PixelCapacity(frame) == (size_t)size->x * size->y * 3 && published.x == size->x && published.y == size->y;
}

int main(void) {
    game_t game = {{0}};
    renderer_framebuffer_t frame = {{0}};
    renderer_raycache_t cache = {{0}};
    numeric_vector_t size = {8, 8, 0, 0};
    numeric_vector_t invalidsize = {0, 8, 0, 0};
    const unsigned char *pixels;

    if (!game_initialize(&game) || !capture_create_frame(&game, &size, &frame) || !frame_valid(&frame, &size)) {
        return 1;
    }
    pixels = renderer_Pixels(&frame);
    if (capture_create_frame(&game, &invalidsize, &frame) || renderer_Pixels(&frame) != pixels || !frame_valid(&frame, &size)) {
        fprintf(stderr, "rejected uncached frame changed the published observation\n");
        return 1;
    }
    if (!capture_create_cached_frame(&cache, &game, &size, &frame) || !frame_valid(&frame, &size) || renderer_CacheEmpty(&cache)) {
        return 1;
    }
    pixels = renderer_Pixels(&frame);
    if (capture_create_cached_frame(&cache, &game, &invalidsize, &frame) || renderer_Pixels(&frame) != pixels || !frame_valid(&frame, &size) || renderer_CacheEmpty(&cache)) {
        fprintf(stderr, "rejected cached frame changed the published observation\n");
        return 1;
    }
    game_destroy(&game);
    if (capture_create_frame(&game, &size, &frame) || capture_create_cached_frame(&cache, &game, &size, &frame) || renderer_Pixels(&frame) != pixels || !frame_valid(&frame, &size) || renderer_CacheEmpty(&cache)) {
        fprintf(stderr, "destroyed game published a frame\n");
        return 1;
    }
    renderer_destroy_framebuffer(&frame);
    renderer_destroy_cache(&cache);
    return 0;
}
