#if defined(__STDC_VERSION__) && __STDC_VERSION__ < 199901L
    #if defined(__linux__)
        #define RGFW_IMPLEMENTATION
        #define RGFW_NO_X11_CURSOR
        #define RGFW_USE_XDL
        #define XDL_NO_GLX
    #else
        #define RGFW_IMPLEMENTATION
    #endif
    #include "../../vendor/rgfw/RGFW.h"
    #include "lib.h"
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ < 202311L
    #if defined(__linux__)
        #define RGFW_IMPLEMENTATION
        #define RGFW_NO_X11_CURSOR
        #define RGFW_USE_XDL
        #define XDL_NO_GLX
    #else
        #define RGFW_IMPLEMENTATION
    #endif
    #include "../../vendor/rgfw/RGFW.h"
    #include "lib.h"
#elif defined(__STDC_VERSION__)
    #if defined(__linux__)
        #define RGFW_IMPLEMENTATION
        #define RGFW_NO_X11_CURSOR
        #define RGFW_USE_XDL
        #define XDL_NO_GLX
    #else
        #define RGFW_IMPLEMENTATION
    #endif
    #include "../../vendor/rgfw/RGFW.h"
    #include "lib.h"
#elif defined(__cplusplus)
    #if defined(__linux__)
        #define RGFW_IMPLEMENTATION
        #define RGFW_NO_X11_CURSOR
        #define RGFW_USE_XDL
        #define XDL_NO_GLX
    #else
        #define RGFW_IMPLEMENTATION
    #endif
    #include "../../vendor/rgfw/RGFW.h"
    #include "lib.h"
#else
    #if defined(__linux__)
        #define RGFW_IMPLEMENTATION
        #define RGFW_NO_X11_CURSOR
        #define RGFW_USE_XDL
        #define XDL_NO_GLX
    #else
        #define RGFW_IMPLEMENTATION
    #endif
    #include "../../vendor/rgfw/RGFW.h"
    #include "lib.h"
#endif

typedef struct {
    void *native;
    void *surface;
} window_state_t;

typedef char window_storage_fits_t[sizeof(window_state_t) <= sizeof(window_t) ? 1 : -1];

static window_state_t *state(window_t *window) {
    return (window_state_t *)window->storage;
}

bool window_initialize(window_t *window, int width, int height) {
    window_state_t *windowstate;

    if (window == NULL || state(window)->native != NULL || width <= 0 || height <= 0) {
        return false;
    }
    windowstate = state(window);
    windowstate->native = RGFW_createWindow("Playme", 0, 0, width, height, RGFW_windowCenter | RGFW_windowNoResize | RGFW_windowCenterCursor);
    if (windowstate->native == NULL) {
        return false;
    }
    RGFW_window_holdMouse((RGFW_window *)windowstate->native);
    return true;
}

bool window_poll(window_t *window, window_input_t *input) {
    window_state_t *windowstate;
    RGFW_window *native;
    RGFW_event event;

    if (window == NULL || input == NULL || state(window)->native == NULL) {
        return false;
    }
    windowstate = state(window);
    native = (RGFW_window *)windowstate->native;
    input->dx = 0;
    input->dy = 0;
    while (RGFW_window_checkEvent(native, &event)) {
        if (event.type == RGFW_mousePosChanged) {
            input->dx += (int)event.mouse.vecX;
            input->dy += (int)event.mouse.vecY;
        }
    }
    input->forward = RGFW_window_isKeyDown(native, RGFW_w) || RGFW_window_isKeyDown(native, RGFW_up);
    input->backward = RGFW_window_isKeyDown(native, RGFW_s) || RGFW_window_isKeyDown(native, RGFW_down);
    input->left = RGFW_window_isKeyDown(native, RGFW_a) || RGFW_window_isKeyDown(native, RGFW_left);
    input->right = RGFW_window_isKeyDown(native, RGFW_d) || RGFW_window_isKeyDown(native, RGFW_right);
    return !RGFW_window_shouldClose(native);
}

bool window_present(window_t *window, unsigned char *pixels, int width, int height) {
    window_state_t *windowstate;

    if (window == NULL || pixels == NULL || state(window)->native == NULL) {
        return false;
    }
    windowstate = state(window);
    if (windowstate->surface == NULL) {
        windowstate->surface = RGFW_window_createSurface((RGFW_window *)windowstate->native, pixels, width, height, RGFW_formatRGB8);
        if (windowstate->surface == NULL) {
            return false;
        }
    }
    RGFW_window_blitSurface((RGFW_window *)windowstate->native, (RGFW_surface *)windowstate->surface);
    return true;
}

void window_destroy(window_t *window) {
    window_state_t *windowstate;

    if (window == NULL) {
        return;
    }
    windowstate = state(window);
    if (windowstate->surface != NULL) {
        RGFW_surface_free((RGFW_surface *)windowstate->surface);
        windowstate->surface = NULL;
    }
    if (windowstate->native != NULL) {
        RGFW_window_close((RGFW_window *)windowstate->native);
        windowstate->native = NULL;
    }
}
