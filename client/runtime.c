#include "../playme/capture/lib.h"

#include "../playme/renderer/lib.h"

#include "../playme/game/lib.h"

#include "../playme/numeric.h"

#include "window/lib.h"

#if defined(__STDC_VERSION__) && __STDC_VERSION__ < 199901L
    #define bool int
    #define true 1
    #define false 0
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ < 202311L
    #include <stdbool.h>
#elif defined(__STDC_VERSION__)
#elif defined(__cplusplus)
#else
    #define bool int
    #define true 1
    #define false 0
#endif

#include <stdio.h>
#include <string.h>
#include "../playme/media/encoded_image.h"
#include "runtime.h"

static int open(void) {
    game_t game = {{0}};
    renderer_framebuffer_t frame = {{0}};
    renderer_raycache_t rays = {{0}};
    numeric_vector_t size = {640, 400, 0, 0};
    window_t window = {{0}};
    window_input_t input;
    numeric_fixed_t distance;
    numeric_fixed_t forward;
    numeric_fixed_t right;
    int yaw = 0;
    int pitch = 0;
    int status = 0;

    if (!window_initialize(&window, size.x, size.y)) {
        fputs("could not create RGFW window\n", stderr);
        return 1;
    }
    if (!game_initialize(&game)) {
        fputs("could not initialize game\n", stderr);
        window_destroy(&window);
        return 1;
    }
    distance = numeric_Ratio(640, 1000);
    while (window_poll(&window, &input)) {
        yaw -= input.dx / 3;
        pitch -= input.dy / 3;
        if (pitch < -56) {
            pitch = -56;
        } else if (56 < pitch) {
            pitch = 56;
        }
        forward = numeric_FixedFromInteger(0);
        right = numeric_FixedFromInteger(0);
        if (input.forward) {
            forward = numeric_Sum(forward, distance);
        }
        if (input.backward) {
            forward = numeric_Difference(forward, distance);
        }
        if (input.right) {
            right = numeric_Sum(right, distance);
        }
        if (input.left) {
            right = numeric_Difference(right, distance);
        }
        if (numeric_RawValue(forward) != 0 && numeric_RawValue(right) != 0) {
            forward = numeric_Product(forward, numeric_Ratio(181, 256));
            right = numeric_Product(right, numeric_Ratio(181, 256));
        }
        game_set_view_rotation(&game, yaw, pitch);
        game_move_player(&game, forward, right);
        if (!capture_create_cached_frame(&rays, &game, &size, &frame)) {
            fprintf(stderr, "could not render frame\n");
            status = 1;
            break;
        }
        if (!window_present(&window, (unsigned char *)renderer_Pixels(&frame), size.x, size.y)) {
            fputs("could not present RGFW surface\n", stderr);
            status = 1;
            break;
        }
    }

    renderer_destroy_cache(&rays);
    renderer_destroy_framebuffer(&frame);
    game_destroy(&game);
    window_destroy(&window);
    return status;
}

static int capture(int nargs, char **args) {
    game_t game = {{0}};
    encoded_image_t image = {{0}};
    numeric_vector_t size = {320, 200, 0, 0};
    game_movement_t movement;
    size_t nchars;
    size_t nwrittenbytes;
    int arg;
    int charind;
    bool encoded;
    bool complete;

    if (strcmp(args[nargs - 1], "jpeg") != 0 && strcmp(args[nargs - 1], "gif") != 0) {
        fprintf(stderr, "usage: %s [wasd ...] <jpeg|gif>\n", args[0]);
        return 1;
    }
    if (!game_initialize(&game)) {
        fprintf(stderr, "could not initialize game\n");
        return 1;
    }
    for (arg = 1; arg + 1 < nargs; arg++) {
        nchars = strlen(args[arg]);
        for (charind = 0; charind < (int)nchars; charind++) {
            movement = game_Movement(args[arg][charind]);
            if (movement == GAME_MOVEMENT_NONE) {
                fprintf(stderr, "unknown movement: %c\n", args[arg][charind]);
                game_destroy(&game);
                return 1;
            }
            game_apply_movement(&game, movement);
        }
    }
    game_set_current_time(&game);
    if (strcmp(args[nargs - 1], "gif") == 0) {
        encoded = capture_create_gif(&game, &size, &image);
    } else {
        encoded = capture_create_jpeg(&game, &size, &image);
    }
    if (!encoded) {
        fprintf(stderr, "could not capture framebuffer\n");
        game_destroy(&game);
        return 1;
    }
    nwrittenbytes = fwrite(encoded_image_data(&image), 1, encoded_image_size(&image), stdout);
    complete = nwrittenbytes == encoded_image_size(&image);
    encoded_image_destroy(&image);
    game_destroy(&game);
    return complete ? 0 : 1;
}

static int run(int nargs, char **args) {
    if (nargs == 1) {
        return open();
    }
    return capture(nargs, args);
}

int playme_client_run(int nargs, char **args) {
    return run(nargs, args);
}
