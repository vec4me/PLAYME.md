#if 0
fail() {
    printf '%s\n' "$*" >&2
    exit 1
}
cd "$(dirname "$0")/../" || fail "could not enter project directory"

sh tools/build_server.sh || fail "could not build Playme library"
${CC:-cc} ${CFLAGS-} "$0" build/gif_c.o build/game_c.o build/encoded_image.o build/capture_c.o build/jpeg_c.o build/media_c.o build/world.o build/numeric_c.o build/renderer_c.o build/rasterizer_c.o build/text_c.o ${LDFLAGS-} ${LDLIBS-} -o build/reject_incomplete_image_encoding_causes_test || fail "could not build incomplete image encoding test"
build/reject_incomplete_image_encoding_causes_test || fail "could not reject incomplete image encoding causes"
exit 0
#endif

#include <stdlib.h>
#include "../playme/media/encoded_image.h"
#include "../playme/media/lib.h"
#include "../playme/numeric.h"

int main(void) {
    encoded_image_t image = {{0}};
    numeric_vector_t size = {1, 1, 0, 0};
    unsigned char pixels[3] = {32, 64, 128};
    unsigned char *sentinel = (unsigned char *)malloc(1);

    if (sentinel == NULL || !encoded_image_adopt(&image, sentinel, 1)) {
        free(sentinel);
        return 1;
    }
    if (media_encode(MEDIA_FORMAT_GIF, NULL, &size, &image) || media_encode(MEDIA_FORMAT_GIF, pixels, NULL, &image) || media_encode(MEDIA_FORMAT_GIF, pixels, &size, NULL) ||
        encoded_image_data(&image) != sentinel || encoded_image_size(&image) != 1) {
        encoded_image_destroy(&image);
        return 1;
    }
    encoded_image_destroy(&image);
    return 0;
}
