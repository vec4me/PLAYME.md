#ifndef PLAYME_CAPTURE_LIB_H
#define PLAYME_CAPTURE_LIB_H

#if defined(__STDC_VERSION__) && __STDC_VERSION__ < 199901L
    #include "../game/lib.h"
    #include "../media/encoded_image.h"
    #include "../numeric.h"
    #include "../renderer/lib.h"
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ < 202311L
    #include "../game/lib.h"
    #include "../media/encoded_image.h"
    #include "../numeric.h"
    #include "../renderer/lib.h"
#elif defined(__STDC_VERSION__)
    #include "../game/lib.h"
    #include "../media/encoded_image.h"
    #include "../numeric.h"
    #include "../renderer/lib.h"
#elif defined(__cplusplus)
    #include "../game/lib.h"
    #include "../media/encoded_image.h"
    #include "../numeric.h"
    #include "../renderer/lib.h"
#else
    #include "../game/lib.h"
    #include "../media/encoded_image.h"
    #include "../numeric.h"
    #include "../renderer/lib.h"
#endif

bool capture_create_frame(const game_t *game, const numeric_vector_t *size, renderer_framebuffer_t *frame);
bool capture_create_cached_frame(renderer_raycache_t *cache, const game_t *game, const numeric_vector_t *size, renderer_framebuffer_t *frame);
bool capture_create_jpeg(const game_t *game, const numeric_vector_t *size, encoded_image_t *output);
bool capture_create_gif(const game_t *game, const numeric_vector_t *size, encoded_image_t *output);
bool capture_create_jpeg_cached(renderer_raycache_t *cache, const game_t *game, const numeric_vector_t *size, encoded_image_t *output);
bool capture_create_gif_cached(renderer_raycache_t *cache, const game_t *game, const numeric_vector_t *size, encoded_image_t *output);

#endif
