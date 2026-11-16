#if 0
fail() {
    printf '%s\n' "$*" >&2
    exit 1
}

cd "$(dirname "$0")/../" || fail "could not enter project directory"

${CC:-cc} ${CFLAGS-} \
    "$0" \
    playme/media/gif.c \
    playme/numeric.c \
    -o build/transfer_the_finished_gif_test || fail "could not build GIF ownership test"

build/transfer_the_finished_gif_test || fail "could not run GIF ownership test"
exit 0
#endif

#include <stdlib.h>
#include "../playme/media/gif.h"
#include "../playme/numeric.h"

int main(void) {
    gif_encoder_t animation = {{0}};
    numeric_vector_t size = {1, 1, 0, 0};
    unsigned char pixels[3] = {0, 0, 0};
    unsigned char *image;
    size_t nbytes;

    if (!gif_begin(&animation, &size) || !gif_write_frame(&animation, pixels, 0) || !gif_finish(&animation, &image, &nbytes)) {
        gif_destroy(&animation);
        return 1;
    }
    if (image == NULL || nbytes == 0 || !gif_empty(&animation)) {
        free(image);
        gif_destroy(&animation);
        return 1;
    }
    free(image);
    gif_destroy(&animation);
    return 0;
}
