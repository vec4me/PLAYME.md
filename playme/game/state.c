#if defined(__STDC_VERSION__) && __STDC_VERSION__ < 199901L
    #include <string.h>
    #include <time.h>
    #include "lib.h"
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ < 202311L
    #include <string.h>
    #include <time.h>
    #include "lib.h"
#elif defined(__STDC_VERSION__)
    #include <string.h>
    #include <time.h>
    #include "lib.h"
#elif defined(__cplusplus)
    #include <string.h>
    #include <time.h>
    #include "lib.h"
#else
    #include <string.h>
    #include <time.h>
    #include "lib.h"
#endif

typedef enum {
    GAME_LIFECYCLE_EMPTY,
    GAME_LIFECYCLE_ACTIVE
} game_lifecycle_t;

typedef struct {
    numeric_fixedvector_t pos;
    numeric_quaternion_t rot;
    numeric_fixed_t speed;
} game_character_t;

typedef struct {
    game_character_t player;
    numeric_fixedvector_t sun;
    unsigned long nmoves;
    int pitch;
    game_lifecycle_t lifecycle;
} game_state_t;

typedef char game_storage_fits_t[sizeof(game_state_t) <= sizeof(game_t) ? 1 : -1];

game_movement_t game_Movement(char action) {
    if (action == 'w') {
        return GAME_MOVEMENT_FORWARD;
    }
    if (action == 's') {
        return GAME_MOVEMENT_BACKWARD;
    }
    if (action == 'a') {
        return GAME_MOVEMENT_TURN_LEFT;
    }
    if (action == 'd') {
        return GAME_MOVEMENT_TURN_RIGHT;
    }
    return GAME_MOVEMENT_NONE;
}

static game_state_t *mutable_state(game_t *game) {
    return (game_state_t *)game->storage;
}

static const game_state_t *constant_state(const game_t *game) {
    return (const game_state_t *)game->storage;
}

static bool active(const game_t *game) {
    return game != NULL && constant_state(game)->lifecycle == GAME_LIFECYCLE_ACTIVE;
}

static void set_sun_time(game_state_t *state, int time) {
    int angle = time % 256;

    if (angle < 0) {
        angle += 256;
    }
    state->sun = numeric_Direction(0, 0, 0);
    state->sun.x = numeric_Sine(angle);
    state->sun.y = numeric_Negative(numeric_Cosine(angle));
}

static game_camera_t Camera(const game_t *game) {
    const game_state_t *state = constant_state(game);
    game_camera_t camera;

    camera.pos = state->player.pos;
    camera.rot = numeric_QuaternionProduct(state->player.rot, numeric_YXZQuaternion(0, state->pitch, 0));
    numeric_normalize_quaternion(&camera.rot);
    return camera;
}

static numeric_fixedvector_t Sun(const game_t *game) {
    return constant_state(game)->sun;
}

static unsigned long MoveCount(const game_t *game) {
    return constant_state(game)->nmoves;
}

bool game_observe_frame(const game_t *game, game_frame_t *frame) {
    if (frame == NULL || !active(game)) {
        return false;
    }
    frame->camera = Camera(game);
    frame->sun = Sun(game);
    frame->nmoves = MoveCount(game);
    return true;
}

void game_set_time(game_t *game, int time) {
    if (!active(game)) {
        return;
    }
    set_sun_time(mutable_state(game), time);
}

void game_set_current_time(game_t *game) {
    time_t now;
    struct tm *utc;

    if (!active(game)) {
        return;
    }
    now = time(NULL);
    utc = gmtime(&now);
    if (utc == NULL) {
        set_sun_time(mutable_state(game), 0);
        return;
    }
    set_sun_time(mutable_state(game), numeric_Turn(((utc->tm_hour + 9) % 24) * 60 + utc->tm_min, 24 * 60));
}

bool game_initialize(game_t *game) {
    game_state_t *state;
    game_state_t initial;

    if (game == NULL || active(game)) {
        return false;
    }
    memset(&initial, 0, sizeof(initial));
    state = mutable_state(game);
    *state = initial;
    state->player.pos = numeric_Point(0, 8, -300);
    state->player.rot = numeric_IdentityQuaternion();
    state->player.speed = numeric_FixedFromInteger(13);
    state->nmoves = 0;
    state->pitch = 0;
    state->lifecycle = GAME_LIFECYCLE_ACTIVE;
    game_set_current_time(game);
    return true;
}

void game_destroy(game_t *game) {
    if (game == NULL) {
        return;
    }
    mutable_state(game)->lifecycle = GAME_LIFECYCLE_EMPTY;
}

void game_apply_movement(game_t *game, game_movement_t movement) {
    game_state_t *state;
    numeric_fixedvector_t translation;
    numeric_matrix_t rotation;
    int angle;

    if (!active(game)) {
        return;
    }
    state = mutable_state(game);
    if (movement == GAME_MOVEMENT_TURN_RIGHT || movement == GAME_MOVEMENT_TURN_LEFT) {
        angle = movement == GAME_MOVEMENT_TURN_RIGHT ? -32 : 32;
        state->player.rot = numeric_QuaternionProduct(state->player.rot, numeric_YXZQuaternion(angle, 0, 0));
        numeric_normalize_quaternion(&state->player.rot);
    } else if (movement == GAME_MOVEMENT_FORWARD) {
        translation = numeric_Direction(0, 0, 0);
        translation.z = numeric_Negative(state->player.speed);
        numeric_set_quaternion(&rotation, state->player.rot);
        numeric_transform(&rotation, &translation);
        state->player.pos.x = numeric_Sum(state->player.pos.x, translation.x);
        state->player.pos.z = numeric_Sum(state->player.pos.z, translation.z);
    } else if (movement == GAME_MOVEMENT_BACKWARD) {
        translation = numeric_Direction(0, 0, 0);
        translation.z = state->player.speed;
        numeric_set_quaternion(&rotation, state->player.rot);
        numeric_transform(&rotation, &translation);
        state->player.pos.x = numeric_Sum(state->player.pos.x, translation.x);
        state->player.pos.z = numeric_Sum(state->player.pos.z, translation.z);
    } else {
        return;
    }
    state->nmoves++;
    state->pitch = 0;
}

void game_move_player(game_t *game, numeric_fixed_t forward, numeric_fixed_t right) {
    game_state_t *state;
    numeric_fixedvector_t translation;
    numeric_matrix_t rotation;

    if (!active(game)) {
        return;
    }
    state = mutable_state(game);
    translation = numeric_Direction(0, 0, 0);
    translation.x = right;
    translation.z = numeric_Negative(forward);
    numeric_set_quaternion(&rotation, state->player.rot);
    numeric_transform(&rotation, &translation);
    state->player.pos.x = numeric_Sum(state->player.pos.x, translation.x);
    state->player.pos.z = numeric_Sum(state->player.pos.z, translation.z);
    if (numeric_RawValue(forward) != 0 || numeric_RawValue(right) != 0) {
        state->nmoves++;
    }
}

void game_set_view_rotation(game_t *game, int yaw, int pitch) {
    game_state_t *state;

    if (!active(game)) {
        return;
    }
    state = mutable_state(game);
    state->player.rot = numeric_YXZQuaternion(yaw, 0, 0);
    numeric_normalize_quaternion(&state->player.rot);
    state->pitch = pitch;
}
