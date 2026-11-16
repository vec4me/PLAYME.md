#if 0
cd "$(dirname "$0")/.." || exit 1
mkdir -p build/ || exit 1
${CC:-cc} ${CFLAGS-} "$0" server/network_channel/channel.c server/network_channel/http.c server/network_channel/socket.c ${LDFLAGS-} ${LDLIBS-} -o build/reject_oversized_redirect_locations_test || exit 1
build/reject_oversized_redirect_locations_test
exit $?
#endif

#include <string.h>

#include "../server/network_channel/internal.h"

int main(void) {
    char location[512];
    network_channel_channel_t channel = {{0}};

    memset(location, 'a', sizeof(location) - 1);
    location[sizeof(location) - 1] = '\0';
    return network_channel_http_redirect(-1, location) || network_channel_redirect(&channel, location) ? 1 : 0;
}
