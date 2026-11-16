#ifndef PLAYME_MEDIA_ENCODED_IMAGE_H
#define PLAYME_MEDIA_ENCODED_IMAGE_H

#if defined(__STDC_VERSION__) && __STDC_VERSION__ < 199901L
    #define bool int
    #define true 1
    #define false 0
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ < 202311L
    #include <stdbool.h>
#elif defined(__STDC_VERSION__)
#elif defined(__cplusplus)
#else
    #define bool int
    #define true 1
    #define false 0
#endif

#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

typedef union {
    unsigned char storage[sizeof(unsigned char *) + sizeof(size_t)];
    void *alignptr;
    size_t alignsize;
} encoded_image_t;

bool encoded_image_adopt(encoded_image_t *image, unsigned char *data, size_t nbytes);
const unsigned char *encoded_image_data(const encoded_image_t *image);
size_t encoded_image_size(const encoded_image_t *image);
void encoded_image_destroy(encoded_image_t *image);

#if defined(__cplusplus)
}
#endif

#endif
