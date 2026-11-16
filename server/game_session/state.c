#if defined(__STDC_VERSION__) && __STDC_VERSION__ < 199901L
    #include "../../playme/capture/lib.h"
    #include "../../playme/game/lib.h"
    #include "../../playme/media/encoded_image.h"
    #include "../../playme/numeric.h"
    #include "../../playme/renderer/lib.h"
    #include "../network_channel/lib.h"
    #include "lib.h"
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ < 202311L
    #include "../../playme/capture/lib.h"
    #include "../../playme/game/lib.h"
    #include "../../playme/media/encoded_image.h"
    #include "../../playme/numeric.h"
    #include "../../playme/renderer/lib.h"
    #include "../network_channel/lib.h"
    #include "lib.h"
#elif defined(__STDC_VERSION__)
    #include "../../playme/capture/lib.h"
    #include "../../playme/game/lib.h"
    #include "../../playme/media/encoded_image.h"
    #include "../../playme/numeric.h"
    #include "../../playme/renderer/lib.h"
    #include "../network_channel/lib.h"
    #include "lib.h"
#elif defined(__cplusplus)
    #include "../../playme/capture/lib.h"
    #include "../../playme/game/lib.h"
    #include "../../playme/media/encoded_image.h"
    #include "../../playme/numeric.h"
    #include "../../playme/renderer/lib.h"
    #include "../network_channel/lib.h"
    #include "lib.h"
#else
    #include "../../playme/capture/lib.h"
    #include "../../playme/game/lib.h"
    #include "../../playme/media/encoded_image.h"
    #include "../../playme/numeric.h"
    #include "../../playme/renderer/lib.h"
    #include "../network_channel/lib.h"
    #include "lib.h"
#endif

typedef enum {
    GAME_SESSION_LIFECYCLE_EMPTY,
    GAME_SESSION_LIFECYCLE_ACTIVE
} game_session_lifecycle_t;

typedef struct {
    game_t game;
    renderer_raycache_t cache;
    game_session_lifecycle_t lifecycle;
} game_session_state_t;

typedef char game_session_storage_fits_t[sizeof(game_session_state_t) <= sizeof(game_session_t) ? 1 : -1];

static game_session_state_t *mutable_state(game_session_t *session) {
    return (game_session_state_t *)session->storage;
}

static const game_session_state_t *constant_state(const game_session_t *session) {
    return (const game_session_state_t *)session->storage;
}

bool game_session_active(const game_session_t *session) {
    return session != NULL && constant_state(session)->lifecycle == GAME_SESSION_LIFECYCLE_ACTIVE;
}

bool game_session_initialize(game_session_t *session) {
    game_session_state_t *state;

    if (session == NULL || game_session_active(session)) {
        return false;
    }
    state = mutable_state(session);
    if (!game_initialize(&state->game)) {
        return false;
    }
    state->lifecycle = GAME_SESSION_LIFECYCLE_ACTIVE;
    return true;
}

static void send_image_response(game_session_state_t *state, const network_channel_channel_t *channel, network_channel_image_media_t media) {
    encoded_image_t image = {{0}};
    numeric_vector_t size = {320, 200, 0, 0};
    bool encoded;

    game_set_current_time(&state->game);
    if (media == NETWORK_CHANNEL_GIF) {
        encoded = capture_create_gif_cached(&state->cache, &state->game, &size, &image);
    } else {
        encoded = capture_create_jpeg_cached(&state->cache, &state->game, &size, &image);
    }
    if (!encoded) {
        network_channel_respond_failure(channel);
        return;
    }
    network_channel_respond_image(channel, media, encoded_image_data(&image), encoded_image_size(&image));
    encoded_image_destroy(&image);
}

void game_session_handle(game_session_t *session, const network_channel_channel_t *channel) {
    game_session_state_t *state;
    char action;

    if (!game_session_active(session) || !network_channel_channel_open(channel)) {
        return;
    }
    state = mutable_state(session);
    action = network_channel_receive_action(channel);
    if (action == '\0') {
        return;
    }
    if (action == 'v') {
        send_image_response(state, channel, NETWORK_CHANNEL_JPEG);
        return;
    }
    if (action == 'g') {
        send_image_response(state, channel, NETWORK_CHANNEL_GIF);
        return;
    }
    game_apply_movement(&state->game, game_Movement(action));
    network_channel_redirect(channel, "https://github.com/vec4me");
}

void game_session_destroy(game_session_t *session) {
    game_session_state_t *state;

    if (!game_session_active(session)) {
        return;
    }
    state = mutable_state(session);
    renderer_destroy_cache(&state->cache);
    game_destroy(&state->game);
    state->lifecycle = GAME_SESSION_LIFECYCLE_EMPTY;
}
