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
    -o build/preserve_framebuffer_state_after_a_rejected_resize_test || fail "could not build renderer invariant test"
build/preserve_framebuffer_state_after_a_rejected_resize_test || fail "could not preserve framebuffer state after a rejected resize"
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
    numeric_vector_t requestsize = {0, 8, 0, 0};
    unsigned char *pixels;
    size_t pixelcap;

    if (!renderer_create_cached(&rays, &pos, &rot, &sun, &size, &frame)) {
        fprintf(stderr, "could not create initial framebuffer\n");
        return 1;
    }
    pixels = (unsigned char *)renderer_Pixels(&frame);
    pixelcap = renderer_PixelCapacity(&frame);
    if (renderer_create_cached(&rays, &pos, &rot, &sun, &requestsize, &frame) || renderer_Pixels(&frame) != pixels || renderer_PixelCapacity(&frame) != pixelcap ||
        renderer_FramebufferSize(&frame).x != size.x || renderer_FramebufferSize(&frame).y != size.y) {
        fprintf(stderr, "failed resize changed framebuffer state\n");
        renderer_destroy_framebuffer(&frame);
        renderer_destroy_cache(&rays);
        return 1;
    }
    renderer_destroy_framebuffer(&frame);
    renderer_destroy_cache(&rays);
    return 0;
}
