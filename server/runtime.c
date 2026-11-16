#include "network_channel/lib.h"
#include "game_session/lib.h"
#include "runtime.h"

static int serve(int port) {
    network_channel_listener_t listener = {{0}};
    network_channel_channel_t channel = {{0}};
    game_session_t session = {{0}};

    if (!network_channel_open(&listener, port)) {
        return 1;
    }

    if (!game_session_initialize(&session)) {
        network_channel_close_listener(&listener);
        return 1;
    }
    for (;;) {
        if (!network_channel_accept(&listener, &channel)) {
            continue;
        }
        game_session_handle(&session, &channel);
        network_channel_close_channel(&channel);
    }
}

int playme_server_run(int port) {
    return serve(port);
}
