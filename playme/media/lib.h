#ifndef PLAYME_MEDIA_LIB_H
#define PLAYME_MEDIA_LIB_H

#include "../numeric.h"
#include "encoded_image.h"

typedef enum {
    MEDIA_FORMAT_GIF,
    MEDIA_FORMAT_JPEG
} media_format_t;

#if defined(__cplusplus)
extern "C" {
#endif

bool media_encode(media_format_t format, const unsigned char *pixels, const numeric_vector_t *size, encoded_image_t *output);

#if defined(__cplusplus)
}
#endif

#endif
