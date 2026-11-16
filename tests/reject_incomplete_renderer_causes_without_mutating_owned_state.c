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
    -o build/reject_incomplete_renderer_causes_without_mutating_owned_state_test || fail "could not build renderer invariant test"
build/reject_incomplete_renderer_causes_without_mutating_owned_state_test || fail "could not reject incomplete renderer causes without mutating owned state"
exit 0
#endif

#include <stddef.h>
#include "../playme/numeric.h"
#include "../playme/renderer/lib.h"


int main(void) {
    renderer_framebuffer_t frame = {{0}};
    renderer_raycache_t cache = {{0}};
    numeric_fixedvector_t pos = numeric_Point(0, 8, -300);
    numeric_quaternion_t rot = numeric_IdentityQuaternion();
    numeric_fixedvector_t sun = numeric_Direction(0, -1, 0);
    numeric_vector_t size = {8, 8, 0, 0};

    if (renderer_create_cached(NULL, &pos, &rot, &sun, &size, &frame) || renderer_create_cached(&cache, NULL, &rot, &sun, &size, &frame) ||
        renderer_create_cached(&cache, &pos, NULL, &sun, &size, &frame) || renderer_create_cached(&cache, &pos, &rot, NULL, &size, &frame) ||
        renderer_create_cached(&cache, &pos, &rot, &sun, NULL, &frame) || renderer_create_cached(&cache, &pos, &rot, &sun, &size, NULL) ||
        renderer_create(NULL, &rot, &sun, &size, &frame) || renderer_create(&pos, NULL, &sun, &size, &frame) || renderer_create(&pos, &rot, NULL, &size, &frame) ||
        renderer_create(&pos, &rot, &sun, NULL, &frame) || renderer_create(&pos, &rot, &sun, &size, NULL) || !renderer_CacheEmpty(&cache) ||
        renderer_Pixels(&frame) != NULL || renderer_PixelCapacity(&frame) != 0) {
        renderer_destroy_framebuffer(&frame);
        renderer_destroy_cache(&cache);
        return 1;
    }
    return 0;
}
