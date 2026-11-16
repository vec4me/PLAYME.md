#if defined(__STDC_VERSION__) && __STDC_VERSION__ < 199901L
    #include <stddef.h>
    #include <stdlib.h>
    #include "gif.h"
    #include "internal.h"
    #include "lib.h"
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ < 202311L
    #include <stddef.h>
    #include <stdlib.h>
    #include "gif.h"
    #include "internal.h"
    #include "lib.h"
#elif defined(__STDC_VERSION__)
    #include <stddef.h>
    #include <stdlib.h>
    #include "gif.h"
    #include "internal.h"
    #include "lib.h"
#elif defined(__cplusplus)
    #include <stddef.h>
    #include <stdlib.h>
    #include "gif.h"
    #include "internal.h"
    #include "lib.h"
#else
    #include <stddef.h>
    #include <stdlib.h>
    #include "gif.h"
    #include "internal.h"
    #include "lib.h"
#endif

bool media_encode(media_format_t format, const unsigned char *pixels, const numeric_vector_t *size, encoded_image_t *output) {
    unsigned char *data = NULL;
    size_t nbytes = 0;
    bool encoded;

    if (pixels == NULL || size == NULL || output == NULL) {
        return false;
    }
    if (format == MEDIA_FORMAT_JPEG) {
        encoded = jpeg_encode(pixels, size, &data, &nbytes);
    } else if (format == MEDIA_FORMAT_GIF) {
        encoded = gif_encode_dithered(pixels, size, &data, &nbytes);
    } else {
        return false;
    }
    if (encoded && encoded_image_adopt(output, data, nbytes)) {
        return true;
    }
    free(data);
    return false;
}
