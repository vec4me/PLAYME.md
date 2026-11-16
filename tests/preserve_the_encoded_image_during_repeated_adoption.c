#if 0
fail() {
    printf '%s\n' "$*" >&2
    exit 1
}
cd "$(dirname "$0")/../" || fail "could not enter project directory"

${CC:-cc} ${CFLAGS-} "$0" playme/media/encoded_image.c ${LDFLAGS-} ${LDLIBS-} -o build/preserve_the_encoded_image_during_repeated_adoption_test || fail "could not build repeated adoption test"
build/preserve_the_encoded_image_during_repeated_adoption_test || fail "could not preserve the encoded image during repeated adoption"
exit 0
#endif

#include <stdlib.h>
#include "../playme/media/encoded_image.h"

int main(void) {
    encoded_image_t image = {{0}};
    unsigned char *data = (unsigned char *)malloc(1);

    if (data == NULL) {
        return 1;
    }
    if (!encoded_image_adopt(&image, data, 1)) {
        free(data);
        return 1;
    }
    if (!encoded_image_adopt(&image, data, 1) || encoded_image_adopt(&image, data, 2) || encoded_image_data(&image) != data || encoded_image_size(&image) != 1) {
        encoded_image_destroy(&image);
        return 1;
    }
    encoded_image_destroy(&image);
    return 0;
}
