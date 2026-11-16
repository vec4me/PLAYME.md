#if 0
fail() {
    printf '%s\n' "$*" >&2
    exit 1
}
cd "$(dirname "$0")/../" || fail "could not enter project directory"

${CC:-cc} ${CFLAGS-} "$0" playme/media/encoded_image.c ${LDFLAGS-} ${LDLIBS-} -o build/empty_the_encoded_image_after_destruction_test || fail "could not build encoded image destruction test"
build/empty_the_encoded_image_after_destruction_test || fail "could not empty the encoded image after destruction"
exit 0
#endif

#include <stdlib.h>
#include "../playme/media/encoded_image.h"

int main(void) {
    encoded_image_t image = {{0}};
    unsigned char *data;
    unsigned char *replacement;
    unsigned char *invalid;

    encoded_image_destroy(NULL);
    if (encoded_image_adopt(NULL, NULL, 0) || encoded_image_data(NULL) != NULL || encoded_image_size(NULL) != 0) {
        return 1;
    }
    data = (unsigned char *)malloc(1);
    if (data == NULL) {
        return 1;
    }
    if (!encoded_image_adopt(&image, data, 1)) {
        free(data);
        return 1;
    }
    if (encoded_image_data(&image) != data || encoded_image_size(&image) != 1) {
        encoded_image_destroy(&image);
        return 1;
    }
    replacement = (unsigned char *)malloc(2);
    if (replacement == NULL) {
        free(replacement);
        encoded_image_destroy(&image);
        return 1;
    }
    if (!encoded_image_adopt(&image, replacement, 2)) {
        free(replacement);
        encoded_image_destroy(&image);
        return 1;
    }
    if (encoded_image_data(&image) != replacement || encoded_image_size(&image) != 2) {
        encoded_image_destroy(&image);
        return 1;
    }
    invalid = (unsigned char *)malloc(1);
    if (invalid == NULL) {
        encoded_image_destroy(&image);
        return 1;
    }
    if (encoded_image_adopt(&image, invalid, 0) || encoded_image_adopt(&image, NULL, 1) || encoded_image_data(&image) != replacement || encoded_image_size(&image) != 2) {
        free(invalid);
        encoded_image_destroy(&image);
        return 1;
    }
    free(invalid);
    encoded_image_destroy(&image);
    encoded_image_destroy(&image);
    if (encoded_image_data(&image) != NULL || encoded_image_size(&image) != 0) {
        return 1;
    }
    return 0;
}
