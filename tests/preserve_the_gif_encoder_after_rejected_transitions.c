#if 0
fail() {
    printf '%s\n' "$*" >&2
    exit 1
}
cd "$(dirname "$0")/../" || fail "could not enter project directory"

mkdir -p build/ || fail "could not create build directory"
${CC:-cc} ${CFLAGS-} \
    "$0" \
    playme/media/gif.c \
    playme/numeric.c \
    -o build/preserve_the_gif_encoder_after_rejected_transitions_test || fail "could not build GIF transition test"
build/preserve_the_gif_encoder_after_rejected_transitions_test || fail "could not preserve the GIF encoder after rejected transitions"
exit 0
#endif

#include <stdlib.h>
#include "../playme/media/gif.h"
#include "../playme/numeric.h"

int main(void) {
    gif_encoder_t encoder = {{0}};
    numeric_vector_t size = {1, 1, 0, 0};
    unsigned char pixels[3] = {0, 0, 0};
    unsigned char *image;
    size_t nbytes;

    gif_destroy(NULL);
    if (!gif_empty(NULL) || gif_begin(NULL, &size) || gif_begin(&encoder, NULL) || gif_write_frame(NULL, pixels, 0) || gif_write_frame(&encoder, NULL, 0) ||
        gif_write_dithered_frame(NULL, pixels, 0) || gif_write_dithered_frame(&encoder, NULL, 0) || gif_finish(NULL, &image, &nbytes) ||
        gif_write_frame(&encoder, pixels, 0) || !gif_empty(&encoder)) {
        return 1;
    }
    if (!gif_begin(&encoder, &size) || gif_begin(&encoder, &size) || gif_empty(&encoder)) {
        gif_destroy(&encoder);
        return 1;
    }
    if (gif_write_frame(&encoder, pixels, -1) || gif_write_frame(&encoder, pixels, 65536) || gif_write_dithered_frame(&encoder, pixels, -1) ||
        gif_write_dithered_frame(&encoder, pixels, 65536) || gif_finish(&encoder, NULL, &nbytes) || gif_finish(&encoder, &image, NULL) ||
        !gif_write_frame(&encoder, pixels, 0) || !gif_finish(&encoder, &image, &nbytes) || !gif_empty(&encoder)) {
        gif_destroy(&encoder);
        return 1;
    }
    free(image);
    if (!gif_begin(&encoder, &size) || !gif_write_dithered_frame(&encoder, pixels, 0) || !gif_finish(&encoder, &image, &nbytes) || !gif_empty(&encoder)) {
        gif_destroy(&encoder);
        return 1;
    }
    free(image);
    if (!gif_begin(&encoder, &size)) {
        return 1;
    }
    gif_destroy(&encoder);
    if (!gif_empty(&encoder) || !gif_begin(&encoder, &size)) {
        gif_destroy(&encoder);
        return 1;
    }
    gif_destroy(&encoder);
    gif_destroy(&encoder);
    return gif_empty(&encoder) ? 0 : 1;
}
