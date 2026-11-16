#ifndef PLAYME_MEDIA_GIF_H
#define PLAYME_MEDIA_GIF_H

#include <stddef.h>

#include "../numeric.h"

typedef union {
    unsigned char storage[sizeof(void *) + 2 * sizeof(size_t) + sizeof(numeric_vector_t) + sizeof(int)];
    void *alignptr;
    size_t alignsize;
} gif_encoder_t;

#if defined(__cplusplus)
extern "C" {
#endif

bool gif_begin(gif_encoder_t *output, const numeric_vector_t *size);
bool gif_write_frame(gif_encoder_t *output, const unsigned char *pixels, int delay);
bool gif_write_dithered_frame(gif_encoder_t *output, const unsigned char *pixels, int delay);
bool gif_finish(gif_encoder_t *output, unsigned char **image, size_t *nbytes);
bool gif_encode(const unsigned char *pixels, const numeric_vector_t *size, unsigned char **image, size_t *nbytes);
bool gif_encode_dithered(const unsigned char *pixels, const numeric_vector_t *size, unsigned char **image, size_t *nbytes);
void gif_destroy(gif_encoder_t *output);
bool gif_empty(const gif_encoder_t *output);

#if defined(__cplusplus)
}
#endif

#endif
