#include <stddef.h>
#include "../playme/media/encoded_image.h"
#include "workload.h"
#include "../playme/numeric.h"
#include "../playme/game/lib.h"
#include "../playme/renderer/lib.h"
#include "../playme/capture/lib.h"

int capture_workload_run(void) {
    game_t game = {{0}};
    encoded_image_t image = {{0}};
    renderer_raycache_t cache = {{0}};
    numeric_vector_t size = {8, 8, 0, 0};
    size_t total = 0;
    int status = 1;
    int iteration;

    if (!game_initialize(&game)) {
        return 1;
    }
    for (iteration = 0; iteration < 250; iteration++) {
        game_apply_movement(&game, GAME_MOVEMENT_TURN_LEFT);
        if (!capture_create_jpeg(&game, &size, &image)) {
            goto cleanup;
        }
        total += encoded_image_size(&image);
        if (!capture_create_gif(&game, &size, &image)) {
            goto cleanup;
        }
        total += encoded_image_size(&image);
        if (!capture_create_jpeg_cached(&cache, &game, &size, &image)) {
            goto cleanup;
        }
        total += encoded_image_size(&image);
        if (!capture_create_gif_cached(&cache, &game, &size, &image)) {
            goto cleanup;
        }
        total += encoded_image_size(&image);
    }
    status = total == 0 ? 1 : 0;

cleanup:
    encoded_image_destroy(&image);
    renderer_destroy_cache(&cache);
    game_destroy(&game);
    return status;
}
