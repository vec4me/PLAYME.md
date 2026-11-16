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
    -o build/preserve_image_ownership_across_capture_transitions_test || fail "could not build capture invariant test"
build/preserve_image_ownership_across_capture_transitions_test || fail "could not preserve image ownership across capture transitions"
exit 0
#endif

#include <stdlib.h>
#include "../playme/media/encoded_image.h"
#include "../playme/numeric.h"
#include "../playme/game/lib.h"
#include "../playme/renderer/lib.h"
#include "../playme/capture/lib.h"


static int image_valid(const encoded_image_t *image) {
    return encoded_image_data(image) != NULL && encoded_image_size(image) != 0;
}

int main(void) {
    game_t game = {{0}};
    encoded_image_t image = {{0}};
    renderer_raycache_t cache = {{0}};
    numeric_vector_t invalidsize = {0, 8, 0, 0};
    numeric_vector_t size = {8, 8, 0, 0};
    unsigned char *sentinel = (unsigned char *)malloc(1);

    if (sentinel == NULL || !encoded_image_adopt(&image, sentinel, 1) || !game_initialize(&game)) {
        free(sentinel);
        return 1;
    }
    if (capture_create_jpeg(&game, &invalidsize, &image) || capture_create_gif(&game, &invalidsize, &image) || capture_create_jpeg_cached(&cache, &game, &invalidsize, &image) ||
        capture_create_gif_cached(&cache, &game, &invalidsize, &image) || encoded_image_data(&image) != sentinel || encoded_image_size(&image) != 1) {
        encoded_image_destroy(&image);
        game_destroy(&game);
        renderer_destroy_cache(&cache);
        return 1;
    }
    if (capture_create_jpeg_cached(NULL, &game, &size, &image) || capture_create_gif_cached(NULL, &game, &size, &image) ||
        capture_create_jpeg_cached(&cache, NULL, &size, &image) || capture_create_gif_cached(&cache, NULL, &size, &image) ||
        capture_create_jpeg_cached(&cache, &game, &size, NULL) || capture_create_gif_cached(&cache, &game, &size, NULL) || !renderer_CacheEmpty(&cache) ||
        encoded_image_data(&image) != sentinel || encoded_image_size(&image) != 1) {
        encoded_image_destroy(&image);
        game_destroy(&game);
        renderer_destroy_cache(&cache);
        return 1;
    }
    if (!capture_create_jpeg(&game, &size, &image) || encoded_image_data(&image) == sentinel || !image_valid(&image)) {
        encoded_image_destroy(&image);
        game_destroy(&game);
        renderer_destroy_cache(&cache);
        return 1;
    }
    if (!capture_create_gif(&game, &size, &image) || !image_valid(&image)) {
        encoded_image_destroy(&image);
        game_destroy(&game);
        renderer_destroy_cache(&cache);
        return 1;
    }
    if (!capture_create_jpeg_cached(&cache, &game, &size, &image) || !image_valid(&image) || renderer_CacheEmpty(&cache)) {
        encoded_image_destroy(&image);
        game_destroy(&game);
        renderer_destroy_cache(&cache);
        return 1;
    }
    if (!capture_create_gif_cached(&cache, &game, &size, &image) || !image_valid(&image) || renderer_CacheEmpty(&cache)) {
        encoded_image_destroy(&image);
        game_destroy(&game);
        renderer_destroy_cache(&cache);
        return 1;
    }
    encoded_image_destroy(&image);
    game_destroy(&game);
    renderer_destroy_cache(&cache);
    return 0;
}
