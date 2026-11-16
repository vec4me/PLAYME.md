#if 0
fail() {
    printf '%s\n' "$*" >&2
    exit 1
}

cd "$(dirname "$0")/../" || fail "could not enter project directory"

sh tools/build_server.sh || fail "could not build shared libraries"
mkdir -p build/ || fail "could not create build directory"

${CC:-cc} ${CFLAGS-} \
    "$0" \
    build/gif_c.o \
    build/game_c.o \
    build/encoded_image.o \
    build/jpeg_c.o \
    build/numeric_c.o \
    build/media_c.o \
    build/world.o \
    build/renderer_c.o \
    build/rasterizer_c.o \
    build/text_c.o \
    -o build/walk_around_and_render_test || fail "could not build walk around renderer test"

cp build/walk_around_and_render_test /tmp/walk_around_and_render || fail "could not stage walk around renderer test"
build/walk_around_and_render_test || fail "could not run walk around renderer test"
exit 0
#endif

#include <stdio.h>
#include <stdlib.h>
#include "../playme/media/gif.h"
#include "../playme/numeric.h"
#include "../playme/game/lib.h"
#include "../playme/renderer/lib.h"


int main(void) {
    game_t game = {{0}};
    game_frame_t observation;
    game_camera_t camera;
    numeric_fixedvector_t sun;
    gif_encoder_t animation = {{0}};
    renderer_framebuffer_t framebuffer = {{0}};
    renderer_raycache_t rays = {{0}};
    numeric_vector_t size = {640, 400, 0, 0};
    numeric_scalar_t camerax;
    numeric_scalar_t cameray;
    numeric_scalar_t cameraz;
    numeric_rotation_t camerarot;
    numeric_fixed_t amount;
    unsigned char *image;
    size_t nbytes;
    FILE *file;
    size_t nwrittenbytes;
    int frame;

    if (!game_initialize(&game)) {
        return 1;
    }
    if (!game_observe_frame(&game, &observation)) {
        return 1;
    }
    camera = observation.camera;
    camerax = numeric_Scalar(camera.pos.x, numeric_FixedFromInteger(50));
    cameray = numeric_Scalar(camera.pos.y, numeric_FixedFromInteger(8));
    cameraz = numeric_Scalar(camera.pos.z, numeric_FixedFromInteger(-360));
    camera.rot = numeric_YXZQuaternion(-21, 0, 0);
    camerarot = numeric_Rotation(camera.rot, numeric_IdentityQuaternion());
    if (!gif_begin(&animation, &size)) {
        fprintf(stderr, "could not begin animation\n");
        return 1;
    }
    for (frame = 0; frame < 12; frame++) {
        amount = numeric_Ratio(frame, 11);
        camera.pos.x = numeric_Value(&camerax, amount);
        camera.pos.y = numeric_Value(&cameray, amount);
        camera.pos.z = numeric_Value(&cameraz, amount);
        camera.rot = numeric_RotationValue(&camerarot, amount);
        game_set_time(&game, numeric_Turn(8 + frame, 24));
        if (!game_observe_frame(&game, &observation)) {
            return 1;
        }
        sun = observation.sun;
        if (!renderer_create_cached(&rays, &camera.pos, &camera.rot, &sun, &size, &framebuffer)) {
            return 1;
        }
        if (!gif_write_dithered_frame(&animation, renderer_Pixels(&framebuffer), 10)) {
            return 1;
        }
    }
    if (!gif_finish(&animation, &image, &nbytes)) {
        return 1;
    }
    file = fopen("/tmp/walk_around_and_render.gif", "wb");
    if (file == NULL) {
        fprintf(stderr, "could not open screenshot file\n");
        free(image);
        gif_destroy(&animation);
        return 1;
    }
    nwrittenbytes = fwrite(image, 1, nbytes, file);
    if (fclose(file) != 0 || nwrittenbytes != nbytes) {
        fprintf(stderr, "could not write screenshot\n");
        free(image);
        gif_destroy(&animation);
        return 1;
    }
    free(image);
    gif_destroy(&animation);
    renderer_destroy_framebuffer(&framebuffer);
    renderer_destroy_cache(&rays);
    game_destroy(&game);
    return 0;
}
