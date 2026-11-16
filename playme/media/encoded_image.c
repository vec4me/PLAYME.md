#include <stdlib.h>

#include "encoded_image.h"

typedef struct {
    unsigned char *data;
    size_t nbytes;
} encoded_image_state_t;

typedef char encoded_image_storage_fits_t[sizeof(encoded_image_state_t) <= sizeof(encoded_image_t) ? 1 : -1];

static encoded_image_state_t *state(encoded_image_t *image) {
    return (encoded_image_state_t *)image->storage;
}

static const encoded_image_state_t *constant_state(const encoded_image_t *image) {
    return (const encoded_image_state_t *)image->storage;
}

static void set_state(encoded_image_state_t *image, unsigned char *data, size_t nbytes) {
    image->data = data;
    image->nbytes = nbytes;
}

bool encoded_image_adopt(encoded_image_t *image, unsigned char *data, size_t nbytes) {
    encoded_image_state_t *owned;

    if (image == NULL || (data == NULL && nbytes != 0) || (data != NULL && nbytes == 0)) {
        return false;
    }
    owned = state(image);
    if (data == owned->data) {
        return nbytes == owned->nbytes;
    }
    free(owned->data);
    set_state(owned, data, nbytes);
    return true;
}

const unsigned char *encoded_image_data(const encoded_image_t *image) {
    return image == NULL ? NULL : constant_state(image)->data;
}

size_t encoded_image_size(const encoded_image_t *image) {
    return image == NULL ? 0 : constant_state(image)->nbytes;
}

void encoded_image_destroy(encoded_image_t *image) {
    encoded_image_state_t *owned;

    if (image == NULL) {
        return;
    }
    owned = state(image);
    free(owned->data);
    set_state(owned, NULL, 0);
}
