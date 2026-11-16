#ifndef PLAYME_RENDERER_LIB_H
#define PLAYME_RENDERER_LIB_H

#include <stddef.h>

#include "../numeric.h"

typedef union {
    unsigned char storage[sizeof(void *)];
    void *alignptr;
} renderer_framebuffer_t;

typedef union {
    unsigned char storage[sizeof(void *)];
    void *alignptr;
} renderer_raycache_t;

#if defined(__cplusplus)
extern "C" {
#endif

void renderer_draw_moves(renderer_framebuffer_t *framebuffer, unsigned long nmoves);
bool renderer_create_cached(renderer_raycache_t *cache, const numeric_fixedvector_t *camerapos, const numeric_quaternion_t *camerarot, const numeric_fixedvector_t *sun,
                            const numeric_vector_t *size, renderer_framebuffer_t *framebuffer);
bool renderer_create(const numeric_fixedvector_t *camerapos, const numeric_quaternion_t *camerarot, const numeric_fixedvector_t *sun, const numeric_vector_t *size,
                     renderer_framebuffer_t *framebuffer);
void renderer_destroy_cache(renderer_raycache_t *cache);
void renderer_destroy_framebuffer(renderer_framebuffer_t *framebuffer);
const unsigned char *renderer_Pixels(const renderer_framebuffer_t *framebuffer);
numeric_vector_t renderer_FramebufferSize(const renderer_framebuffer_t *framebuffer);
size_t renderer_PixelCapacity(const renderer_framebuffer_t *framebuffer);
bool renderer_CacheEmpty(const renderer_raycache_t *cache);

#if defined(__cplusplus)
}
#endif

#endif
