#ifndef CLIENT_WINDOW_LIB_H
#define CLIENT_WINDOW_LIB_H

#if defined(__cplusplus)
#elif defined(__STDC_VERSION__)
    #define bool int
    #define true 1
    #define false 0
#else
    #define bool int
    #define true 1
    #define false 0
#endif

typedef union {
    unsigned char storage[2 * sizeof(void *)];
    void *alignptr;
} window_t;

typedef struct {
    int dx;
    int dy;
    bool forward;
    bool backward;
    bool left;
    bool right;
} window_input_t;

bool window_initialize(window_t *window, int width, int height);
bool window_poll(window_t *window, window_input_t *input);
bool window_present(window_t *window, unsigned char *pixels, int width, int height);
void window_destroy(window_t *window);

#endif
