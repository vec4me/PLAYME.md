#if 0
fail() {
    printf '%s\n' "$*" >&2
    exit 1
}
cd "$(dirname "$0")/../" || fail "could not enter project directory"

mkdir -p build/ || fail "could not create build directory"
${CC:-cc} ${CFLAGS-} \
    "$0" \
    playme/media/encoded_image.c \
    playme/media/gif.c \
    playme/media/jpeg.c \
    playme/media/encoding.c \
    playme/numeric.c \
    -o build/reject_incomplete_media_causes_test || fail "could not build media cause test"
build/reject_incomplete_media_causes_test || fail "could not reject incomplete media causes"
exit 0
#endif

#include <stddef.h>
#include <stdlib.h>

#include "../playme/media/lib.h"

int main(void) {
    numeric_vector_t size = {1, 1, 0, 0};
    numeric_vector_t invalidsize = {0, 1, 0, 0};
    unsigned char pixels[3] = {0, 0, 0};
    encoded_image_t image = {{0}};
    unsigned char *sentinel = (unsigned char *)malloc(1);

    if (sentinel == NULL || !encoded_image_adopt(&image, sentinel, 1)) {
        return 1;
    }
    if (media_encode(MEDIA_FORMAT_JPEG, NULL, &size, &image) || media_encode(MEDIA_FORMAT_JPEG, pixels, NULL, &image) ||
        media_encode(MEDIA_FORMAT_JPEG, pixels, &size, NULL) || media_encode((media_format_t)99, pixels, &size, &image) ||
        media_encode(MEDIA_FORMAT_JPEG, pixels, &invalidsize, &image) || encoded_image_data(&image) != sentinel || encoded_image_size(&image) != 1) {
        encoded_image_destroy(&image);
        return 1;
    }
    if (!media_encode(MEDIA_FORMAT_JPEG, pixels, &size, &image) || encoded_image_data(&image) == NULL || encoded_image_data(&image) == sentinel || encoded_image_size(&image) < 4) {
        encoded_image_destroy(&image);
        return 1;
    }
    encoded_image_destroy(&image);
    return 0;
}
