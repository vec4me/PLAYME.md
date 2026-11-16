#if defined(__STDC_VERSION__) && __STDC_VERSION__ < 199901L
    #include <stddef.h>
    #include "../game/lib.h"
    #include "../media/encoded_image.h"
    #include "../media/lib.h"
    #include "../numeric.h"
    #include "../renderer/lib.h"
    #include "lib.h"
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ < 202311L
    #include <stddef.h>
    #include "../game/lib.h"
    #include "../media/encoded_image.h"
    #include "../media/lib.h"
    #include "../numeric.h"
    #include "../renderer/lib.h"
    #include "lib.h"
#elif defined(__STDC_VERSION__)
    #include <stddef.h>
    #include "../game/lib.h"
    #include "../media/encoded_image.h"
    #include "../media/lib.h"
    #include "../numeric.h"
    #include "../renderer/lib.h"
    #include "lib.h"
#elif defined(__cplusplus)
    #include <stddef.h>
    #include "../game/lib.h"
    #include "../media/encoded_image.h"
    #include "../media/lib.h"
    #include "../numeric.h"
    #include "../renderer/lib.h"
    #include "lib.h"
#else
    #include <stddef.h>
    #include "../game/lib.h"
    #include "../media/encoded_image.h"
    #include "../media/lib.h"
    #include "../numeric.h"
    #include "../renderer/lib.h"
    #include "lib.h"
#endif

static bool render_frame(renderer_raycache_t *cache, const game_t *game, const numeric_vector_t *size, renderer_framebuffer_t *frame) {
    game_frame_t observation;
    bool created;

    if (game == NULL || size == NULL || frame == NULL) {
        return false;
    }
    if (!game_observe_frame(game, &observation)) {
        return false;
    }
    created = cache == NULL ? renderer_create(&observation.camera.pos, &observation.camera.rot, &observation.sun, size, frame)
                            : renderer_create_cached(cache, &observation.camera.pos, &observation.camera.rot, &observation.sun, size, frame);
    if (!created) {
        return false;
    }
    renderer_draw_moves(frame, observation.nmoves);
    return true;
}

bool capture_create_frame(const game_t *game, const numeric_vector_t *size, renderer_framebuffer_t *frame) {
    return render_frame(NULL, game, size, frame);
}

bool capture_create_cached_frame(renderer_raycache_t *cache, const game_t *game, const numeric_vector_t *size, renderer_framebuffer_t *frame) {
    if (cache == NULL) {
        return false;
    }
    return render_frame(cache, game, size, frame);
}

static bool capture_image(renderer_raycache_t *cache, const game_t *game, const numeric_vector_t *size, media_format_t format, encoded_image_t *output) {
    renderer_framebuffer_t frame = {{0}};
    numeric_vector_t framesize;
    bool encoded;

    if (output == NULL || !render_frame(cache, game, size, &frame)) {
        return false;
    }
    framesize = renderer_FramebufferSize(&frame);
    encoded = media_encode(format, renderer_Pixels(&frame), &framesize, output);
    renderer_destroy_framebuffer(&frame);
    return encoded;
}

bool capture_create_jpeg(const game_t *game, const numeric_vector_t *size, encoded_image_t *output) {
    return capture_image(NULL, game, size, MEDIA_FORMAT_JPEG, output);
}

bool capture_create_gif(const game_t *game, const numeric_vector_t *size, encoded_image_t *output) {
    return capture_image(NULL, game, size, MEDIA_FORMAT_GIF, output);
}

bool capture_create_jpeg_cached(renderer_raycache_t *cache, const game_t *game, const numeric_vector_t *size, encoded_image_t *output) {
    return cache != NULL && capture_image(cache, game, size, MEDIA_FORMAT_JPEG, output);
}

bool capture_create_gif_cached(renderer_raycache_t *cache, const game_t *game, const numeric_vector_t *size, encoded_image_t *output) {
    return cache != NULL && capture_image(cache, game, size, MEDIA_FORMAT_GIF, output);
}
