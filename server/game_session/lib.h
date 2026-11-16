#ifndef SERVER_GAME_SESSION_LIB_H
#define SERVER_GAME_SESSION_LIB_H

#include <stddef.h>

#include "../../playme/game/lib.h"
#include "../../playme/renderer/lib.h"
#include "../network_channel/lib.h"

typedef union {
    unsigned char storage[sizeof(game_t) + sizeof(renderer_raycache_t) + sizeof(int)];
    void *alignptr;
    long double alignnumber;
} game_session_t;

#if defined(__cplusplus)
extern "C" {
#endif

bool game_session_active(const game_session_t *session);
bool game_session_initialize(game_session_t *session);
void game_session_handle(game_session_t *session, const network_channel_channel_t *channel);
void game_session_destroy(game_session_t *session);

#if defined(__cplusplus)
}
#endif

#endif
