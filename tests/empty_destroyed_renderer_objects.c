#if 0
fail() {
    printf '%s\n' "$*" >&2
    exit 1
}
cd "$(dirname "$0")/../" || fail "could not enter project directory"

mkdir -p build/ || fail "could not create build directory"
${CC:-cc} ${CFLAGS-} \
    "$0" \
    playme/numeric.c \
    playme/renderer/frame.c \
    playme/renderer/rasterizer.c \
    playme/renderer/text.c \
    playme/renderer/world.c \
    -o build/empty_destroyed_renderer_objects_test || fail "could not build renderer invariant test"
build/empty_destroyed_renderer_objects_test || fail "could not empty destroyed renderer objects"
exit 0
#endif

#include <stdio.h>
#include "../playme/numeric.h"
#include "../playme/renderer/lib.h"


int main(void) {
    renderer_framebuffer_t frame = {{0}};
    renderer_raycache_t rays = {{0}};
    numeric_fixedvector_t pos = numeric_Point(0, 8, -300);
    numeric_quaternion_t rot = numeric_IdentityQuaternion();
    numeric_fixedvector_t sun = numeric_Direction(0, -1, 0);
    numeric_vector_t size = {8, 8, 0, 0};

    renderer_destroy_framebuffer(NULL);
    renderer_destroy_cache(NULL);
    renderer_draw_moves(NULL, 1);
    if (renderer_Pixels(NULL) != NULL || renderer_PixelCapacity(NULL) != 0 || renderer_FramebufferSize(NULL).x != 0 || !renderer_CacheEmpty(NULL)) {
        return 1;
    }
    if (!renderer_create_cached(&rays, &pos, &rot, &sun, &size, &frame)) {
        fprintf(stderr, "could not create renderer objects\n");
        return 1;
    }
    renderer_destroy_framebuffer(&frame);
    renderer_destroy_cache(&rays);
    renderer_draw_moves(&frame, 1);
    if (renderer_Pixels(&frame) != NULL || renderer_PixelCapacity(&frame) != 0 || renderer_FramebufferSize(&frame).x != 0 || !renderer_CacheEmpty(&rays)) {
        fprintf(stderr, "destroyed renderer objects retained state\n");
        return 1;
    }
    return 0;
}
