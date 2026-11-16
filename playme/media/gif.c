#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "gif.h"

static const signed char bayer8[64][2] = {
    {-32, -16}, {16, 8},   {-20, -10}, {28, 14},  {-29, -14}, {19, 9},   {-17, -8},  {31, 15},  {0, 0},      {-16, -8}, {12, 6},
    {-4, -2},   {3, 1},    {-13, -6},  {15, 7},   {-1, 0},    {-24, -12}, {24, 12},   {-28, -14}, {20, 10},   {-21, -10}, {27, 13},
    {-25, -12}, {23, 11},  {8, 4},     {-8, -4},  {4, 2},     {-12, -6},  {11, 5},    {-5, -2},   {7, 3},     {-9, -4},   {-30, -15},
    {18, 9},    {-18, -9}, {30, 15},   {-31, -15}, {17, 8},   {-19, -9},  {29, 14},   {2, 1},     {-14, -7},  {14, 7},    {-2, -1},
    {1, 0},     {-15, -7}, {13, 6},    {-3, -1},  {-22, -11}, {26, 13},  {-26, -13}, {22, 11},   {-23, -11}, {25, 12},   {-27, -13},
    {21, 10},   {10, 5},   {-6, -3},   {6, 3},    {-10, -5}, {9, 4},    {-7, -3},   {5, 2},     {-11, -5}};

typedef enum {
    GIF_LIFECYCLE_EMPTY,
    GIF_LIFECYCLE_WRITING
} lifecycle_t;

typedef enum {
    GIF_FRAME_PLAIN,
    GIF_FRAME_DITHERED
} frameencoding_t;

typedef struct {
    unsigned char *data;
    size_t nbytes;
    size_t bytecap;
    numeric_vector_t imagesize;
    lifecycle_t lifecycle;
} encoderstate_t;

typedef char encoder_storage_fits_t[sizeof(encoderstate_t) <= sizeof(gif_encoder_t) ? 1 : -1];

static encoderstate_t *encoder_state(gif_encoder_t *output) {
    return (encoderstate_t *)output->storage;
}

static const encoderstate_t *constant_encoder_state(const gif_encoder_t *output) {
    return (const encoderstate_t *)output->storage;
}

typedef struct {
    unsigned char *data;
    size_t pos;
    size_t blockstart;
    size_t nblockbytes;
    unsigned bits;
    int nbits;
} bitstream_t;

static bool grow(encoderstate_t *output, size_t nbytes) {
    unsigned char *data;
    size_t nrequiredbytes;
    size_t bytecap;

    if (((size_t)-1) - output->nbytes < nbytes) {
        return false;
    }
    nrequiredbytes = output->nbytes + nbytes;
    if (nrequiredbytes <= output->bytecap) {
        return true;
    }
    bytecap = output->bytecap == 0 ? 4096 : output->bytecap;
    while (bytecap < nrequiredbytes) {
        if (((size_t)-1) / 2 < bytecap) {
            bytecap = nrequiredbytes;
            break;
        }
        bytecap *= 2;
    }
    data = (unsigned char *)realloc(output->data, bytecap);
    if (data == NULL) {
        return false;
    }
    output->data = data;
    output->bytecap = bytecap;
    return true;
}

static bool write_byte(encoderstate_t *output, unsigned byte) {
    if (!grow(output, 1)) {
        return false;
    }
    output->data[output->nbytes++] = (unsigned char)byte;
    return true;
}

static bool write_data(encoderstate_t *output, const unsigned char *data, size_t nbytes) {
    if (!grow(output, nbytes)) {
        return false;
    }
    memcpy(output->data + output->nbytes, data, nbytes);
    output->nbytes += nbytes;
    return true;
}

static bool write_word(encoderstate_t *output, unsigned word) {
    return write_byte(output, word) && write_byte(output, word >> 8);
}

static bool write_text(encoderstate_t *output, const char *text, size_t nbytes) {
    return write_data(output, (const unsigned char *)text, nbytes);
}

static void flush_block(bitstream_t *output) {
    if (output->nblockbytes == 0) {
        return;
    }
    output->data[output->blockstart] = (unsigned char)output->nblockbytes;
    output->nblockbytes = 0;
}

static void write_compressed_byte(bitstream_t *output, unsigned byte) {
    if (output->nblockbytes == 0) {
        output->blockstart = output->pos;
        output->data[output->pos++] = 0;
    }
    output->data[output->pos++] = (unsigned char)byte;
    output->nblockbytes++;
    if (output->nblockbytes == 255) {
        output->data[output->blockstart] = 255;
        output->nblockbytes = 0;
    }
}

static void write_code(bitstream_t *output, unsigned code) {
    output->bits |= code << output->nbits;
    write_compressed_byte(output, output->bits);
    output->bits >>= 8;
    output->nbits++;
    if (output->nbits == 8) {
        write_compressed_byte(output, output->bits);
        output->bits = 0;
        output->nbits = 0;
    }
}

static void flush_bits(bitstream_t *output) {
    if (output->nbits != 0) {
        write_compressed_byte(output, output->bits);
    }
}

static bool write_palette(encoderstate_t *output, const unsigned char *palette) {
    return write_data(output, palette, 256 * 3);
}

static bool write_pixels(encoderstate_t *output, const unsigned char *pixels, const numeric_vector_t *size) {
    bitstream_t compressed = {NULL, 0, 0, 0, 0, 0};
    size_t npixels = (size_t)size->x * size->y;
    size_t pixelstart = 0;
    size_t ngrouppixels;
    size_t grouppixel;
    const unsigned char *pixel = pixels;

    compressed.data = output->data;
    compressed.pos = output->nbytes;
    write_code(&compressed, 256);
    while (pixelstart < npixels) {
        ngrouppixels = npixels - pixelstart;

        if (250 < ngrouppixels) {
            ngrouppixels = 250;
        }
        for (grouppixel = 0; grouppixel < ngrouppixels; grouppixel++, pixel += 3) {
            write_code(&compressed, (unsigned)((pixel[0] & 0xe0) | ((pixel[1] & 0xe0) >> 3) | (pixel[2] >> 6)));
        }
        pixelstart += ngrouppixels;
        if (pixelstart < npixels) {
            write_code(&compressed, 256);
        }
    }
    write_code(&compressed, 257);
    flush_bits(&compressed);
    flush_block(&compressed);
    output->nbytes = compressed.pos;
    return write_byte(output, 0);
}

static bool write_dithered_pixels(encoderstate_t *output, const unsigned char *pixels, const numeric_vector_t *size) {
    bitstream_t compressed = {NULL, 0, 0, 0, 0, 0};
    int nremainingpixels = 250;
    int segmentend;
    int nsegmentpixels;
    int threshold;
    int red;
    int green;
    int blue;
    int level;
    int x;
    int y;
    unsigned char quantize3[319];
    unsigned char quantizegreen[319];
    unsigned char quantize2[319];
    const unsigned char *pixel = pixels;
    const signed char (*thresholdrow)[2];

    compressed.data = output->data;
    compressed.pos = output->nbytes;
    write_code(&compressed, 256);
    for (level = -32; level <= 286; level++) {
        int color = level < 0 ? 0 : 255 < level ? 255 : level;

        quantize3[level + 32] = (unsigned char)(color & 0xe0);
        quantizegreen[level + 32] = (unsigned char)((color & 0xe0) >> 3);
        quantize2[level + 32] = (unsigned char)(color >> 6);
    }
    for (y = 0; y < size->y; y++) {
        thresholdrow = bayer8 + (y & 7) * 8;
        x = 0;
        while (x < size->x) {
            nsegmentpixels = nremainingpixels;
            if (size->x - x < nsegmentpixels) {
                nsegmentpixels = size->x - x;
            }
            segmentend = x + nsegmentpixels;
            for (; x < segmentend; x++, pixel += 3) {
                threshold = thresholdrow[x & 7][0];
                red = quantize3[pixel[0] + thresholdrow[x & 7][1] + 32];
                green = quantizegreen[pixel[1] + thresholdrow[x & 7][1] + 32];
                blue = quantize2[pixel[2] + threshold + 32];
                write_code(&compressed, (unsigned)(red | green | blue));
            }
            nremainingpixels -= nsegmentpixels;
            if (nremainingpixels == 0 && (x < size->x || y + 1 < size->y)) {
                write_code(&compressed, 256);
                nremainingpixels = 250;
            }
        }
    }
    write_code(&compressed, 257);
    flush_bits(&compressed);
    flush_block(&compressed);
    output->nbytes = compressed.pos;
    return write_byte(output, 0);
}

static void set_empty(encoderstate_t *state) {
    state->data = NULL;
    state->nbytes = 0;
    state->bytecap = 0;
    state->lifecycle = GIF_LIFECYCLE_EMPTY;
}

bool gif_begin(gif_encoder_t *output, const numeric_vector_t *size) {
    encoderstate_t *state;
    unsigned char palette[256 * 3];
    size_t color;

    if (output == NULL || size == NULL) {
        return false;
    }
    state = encoder_state(output);
    if (state->lifecycle == GIF_LIFECYCLE_WRITING || size->x <= 0 || size->y <= 0 || 65535 < size->x || 65535 < size->y) {
        return false;
    }
    state->imagesize = *size;
    state->lifecycle = GIF_LIFECYCLE_WRITING;
    for (color = 0; color < 256; color++) {
        palette[color * 3] = (unsigned char)(((color >> 5) & 7) * 255 / 7);
        palette[color * 3 + 1] = (unsigned char)(((color >> 2) & 7) * 255 / 7);
        palette[color * 3 + 2] = (unsigned char)((color & 3) * 255 / 3);
    }
    if (!write_text(state, "GIF89a", 6) || !write_word(state, (unsigned)size->x) || !write_word(state, (unsigned)size->y) || !write_byte(state, 0xf7) || !write_byte(state, 0) ||
        !write_byte(state, 0) || !write_palette(state, palette) || !write_byte(state, 0x21) || !write_byte(state, 0xff) || !write_byte(state, 11) ||
        !write_text(state, "NETSCAPE2.0", 11) || !write_byte(state, 3) || !write_byte(state, 1) || !write_word(state, 0) || !write_byte(state, 0)) {
        free(state->data);
        set_empty(state);
        return false;
    }
    return true;
}

static bool write_frame_header(encoderstate_t *state, int delay) {
    size_t npixels;
    size_t ncodes;
    size_t ncompressedbytes;
    size_t nblocks;

    if (state->lifecycle != GIF_LIFECYCLE_WRITING) {
        return false;
    }
    npixels = (size_t)state->imagesize.x * state->imagesize.y;
    ncodes = npixels + (npixels + 249) / 250 + 1;
    ncompressedbytes = (ncodes * 9 + 7) / 8;
    nblocks = (ncompressedbytes + 254) / 255;

    if (!grow(state, 32 + ncompressedbytes + nblocks)) {
        return false;
    }
    if (!write_byte(state, 0x21) || !write_byte(state, 0xf9) || !write_byte(state, 4) || !write_byte(state, 0) || !write_word(state, (unsigned)delay) || !write_byte(state, 0) ||
        !write_byte(state, 0)) {
        return false;
    }
    return write_byte(state, 0x2c) && write_word(state, 0) && write_word(state, 0) && write_word(state, (unsigned)state->imagesize.x) && write_word(state, (unsigned)state->imagesize.y) &&
           write_byte(state, 0) && write_byte(state, 8);
}

static bool write_frame_pixels(gif_encoder_t *output, const unsigned char *pixels, int delay, frameencoding_t encoding) {
    encoderstate_t *state;

    if (output == NULL || pixels == NULL || delay < 0 || 65535 < delay) {
        return false;
    }
    state = encoder_state(output);
    if (!write_frame_header(state, delay)) {
        return false;
    }
    if (encoding == GIF_FRAME_DITHERED) {
        return write_dithered_pixels(state, pixels, &state->imagesize);
    }
    return write_pixels(state, pixels, &state->imagesize);
}

bool gif_write_frame(gif_encoder_t *output, const unsigned char *pixels, int delay) {
    return write_frame_pixels(output, pixels, delay, GIF_FRAME_PLAIN);
}

bool gif_write_dithered_frame(gif_encoder_t *output, const unsigned char *pixels, int delay) {
    return write_frame_pixels(output, pixels, delay, GIF_FRAME_DITHERED);
}

bool gif_finish(gif_encoder_t *output, unsigned char **image, size_t *nbytes) {
    encoderstate_t *state;

    if (output == NULL || image == NULL || nbytes == NULL) {
        return false;
    }
    state = encoder_state(output);
    if (state->lifecycle != GIF_LIFECYCLE_WRITING || !write_byte(state, 0x3b)) {
        return false;
    }
    *image = state->data;
    *nbytes = state->nbytes;
    set_empty(state);
    return true;
}

static void release(gif_encoder_t *output) {
    encoderstate_t *state = encoder_state(output);

    if (state->lifecycle != GIF_LIFECYCLE_WRITING) {
        return;
    }
    free(state->data);
    set_empty(state);
}

bool gif_encode(const unsigned char *pixels, const numeric_vector_t *size, unsigned char **image, size_t *nbytes) {
    gif_encoder_t output = {{0}};
    bool succeeded;

    if (pixels == NULL || size == NULL || image == NULL || nbytes == NULL) {
        return false;
    }
    succeeded = gif_begin(&output, size) && gif_write_frame(&output, pixels, 0) && gif_finish(&output, image, nbytes);
    if (!succeeded) {
        release(&output);
    }
    return succeeded;
}

bool gif_encode_dithered(const unsigned char *pixels, const numeric_vector_t *size, unsigned char **image, size_t *nbytes) {
    gif_encoder_t output = {{0}};
    bool succeeded;

    if (pixels == NULL || size == NULL || image == NULL || nbytes == NULL) {
        return false;
    }
    succeeded = gif_begin(&output, size) && gif_write_dithered_frame(&output, pixels, 0) && gif_finish(&output, image, nbytes);
    if (!succeeded) {
        release(&output);
    }
    return succeeded;
}

void gif_destroy(gif_encoder_t *output) {
    if (output == NULL) {
        return;
    }
    release(output);
}

bool gif_empty(const gif_encoder_t *output) {
    return output == NULL || constant_encoder_state(output)->lifecycle == GIF_LIFECYCLE_EMPTY;
}

