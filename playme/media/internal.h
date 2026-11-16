#ifndef PLAYME_MEDIA_INTERNAL_H
#define PLAYME_MEDIA_INTERNAL_H

#include <stddef.h>

#include "../numeric.h"

bool jpeg_encode(const unsigned char *pixels, const numeric_vector_t *size, unsigned char **image, size_t *nbytes);

#endif
