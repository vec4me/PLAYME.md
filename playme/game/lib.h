#ifndef PLAYME_GAME_LIB_H
#define PLAYME_GAME_LIB_H

#include "../numeric.h"

typedef enum {
    GAME_MOVEMENT_NONE,
    GAME_MOVEMENT_FORWARD,
    GAME_MOVEMENT_BACKWARD,
    GAME_MOVEMENT_TURN_LEFT,
    GAME_MOVEMENT_TURN_RIGHT
} game_movement_t;

typedef union {
    unsigned char storage[2 * sizeof(numeric_fixedvector_t) + sizeof(numeric_quaternion_t) + sizeof(numeric_fixed_t) + sizeof(unsigned long) + 2 * sizeof(int)];
    void *alignptr;
    long double alignnumber;
} game_t;

typedef struct {
    numeric_fixedvector_t pos;
    numeric_quaternion_t rot;
} game_camera_t;

typedef struct {
    game_camera_t camera;
    numeric_fixedvector_t sun;
    unsigned long nmoves;
} game_frame_t;

#if defined(__cplusplus)
extern "C" {
#endif

game_movement_t game_Movement(char action);
bool game_observe_frame(const game_t *game, game_frame_t *frame);
void game_set_time(game_t *game, int time);
void game_set_current_time(game_t *game);
bool game_initialize(game_t *game);
void game_destroy(game_t *game);
void game_apply_movement(game_t *game, game_movement_t movement);
void game_move_player(game_t *game, numeric_fixed_t forward, numeric_fixed_t right);
void game_set_view_rotation(game_t *game, int yaw, int pitch);

#if defined(__cplusplus)
}
#endif

#endif
