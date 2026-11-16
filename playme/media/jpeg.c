#include <stddef.h>
#include <stdlib.h>

#include "internal.h"

typedef struct {
    unsigned char *data;
    size_t nbytes;
    size_t bytecap;
    unsigned char bits;
    int nbits;
} writer_t;

static int Category(int diff) {
    unsigned magnitude = diff < 0 ? (unsigned)-diff : (unsigned)diff;
    int nbits = 0;

    while (magnitude != 0) {
        magnitude >>= 1;
        nbits++;
    }
    return nbits;
}

static bool grow(writer_t *output) {
    unsigned char *data;
    size_t bytecap;

    if (output->bytecap == ((size_t)-1)) {
        return false;
    }
    bytecap = output->bytecap == 0 ? 4096 : output->bytecap;
    bytecap = ((size_t)-1) / 2 < bytecap ? ((size_t)-1) : bytecap * 2;
    data = (unsigned char *)realloc(output->data, bytecap);
    if (data == NULL) {
        return false;
    }
    output->data = data;
    output->bytecap = bytecap;
    return true;
}

static bool write_byte(writer_t *output, unsigned byte) {
    if (output->nbytes == output->bytecap && !grow(output)) {
        return false;
    }
    output->data[output->nbytes++] = (unsigned char)byte;
    return true;
}

static bool write_word(writer_t *output, unsigned word) {
    return write_byte(output, word >> 8) && write_byte(output, word);
}

static bool write_marker(writer_t *output, unsigned marker) {
    return write_byte(output, 0xff) && write_byte(output, marker);
}

static bool write_scan_byte(writer_t *output, unsigned byte) {
    if (!write_byte(output, byte)) {
        return false;
    }
    return byte != 0xff || write_byte(output, 0);
}

static bool write_bits(writer_t *output, unsigned bits, int nbits) {
    int bit = nbits - 1;

    while (0 <= bit) {
        output->bits = (unsigned char)((output->bits << 1) | ((bits >> bit) & 1));
        output->nbits++;
        if (output->nbits == 8) {
            if (!write_scan_byte(output, output->bits)) {
                return false;
            }
            output->bits = 0;
            output->nbits = 0;
        }
        bit--;
    }
    return true;
}

static bool write_block(writer_t *output, int avg, int *prevavg) {
    int diff = avg - *prevavg;
    int nbits = Category(diff);
    unsigned amplitude;

    *prevavg = avg;
    if (!write_bits(output, (unsigned)nbits, 4)) {
        return false;
    }
    if (nbits != 0) {
        amplitude = diff < 0 ? (unsigned)((1 << nbits) - 1 + diff) : (unsigned)diff;
        if (!write_bits(output, amplitude, nbits)) {
            return false;
        }
    }
    return write_bits(output, 0, 1);
}

static bool write_headers(writer_t *output, const numeric_vector_t *size) {
    int quant;
    int component;
    int nbits;
    int category;

    if (!write_marker(output, 0xd8) || !write_marker(output, 0xe0) || !write_word(output, 16)) {
        return false;
    }
    if (!write_byte(output, 'J') || !write_byte(output, 'F') || !write_byte(output, 'I') || !write_byte(output, 'F') || !write_byte(output, 0) || !write_byte(output, 1) ||
        !write_byte(output, 1) || !write_byte(output, 0) || !write_word(output, 1) || !write_word(output, 1) || !write_byte(output, 0) || !write_byte(output, 0)) {
        return false;
    }
    if (!write_marker(output, 0xdb) || !write_word(output, 67) || !write_byte(output, 0)) {
        return false;
    }
    for (quant = 0; quant < 64; quant++) {
        if (!write_byte(output, 8)) {
            return false;
        }
    }
    if (!write_marker(output, 0xc0) || !write_word(output, 17) || !write_byte(output, 8) || !write_word(output, (unsigned)size->y) || !write_word(output, (unsigned)size->x) ||
        !write_byte(output, 3)) {
        return false;
    }
    for (component = 1; component <= 3; component++) {
        if (!write_byte(output, (unsigned)component) || !write_byte(output, 0x11) || !write_byte(output, 0)) {
            return false;
        }
    }
    if (!write_marker(output, 0xc4) || !write_word(output, 49) || !write_byte(output, 0)) {
        return false;
    }
    for (nbits = 1; nbits <= 16; nbits++) {
        if (!write_byte(output, nbits == 4 ? 12 : 0)) {
            return false;
        }
    }
    for (category = 0; category < 12; category++) {
        if (!write_byte(output, (unsigned)category)) {
            return false;
        }
    }
    if (!write_byte(output, 0x10)) {
        return false;
    }
    for (nbits = 1; nbits <= 16; nbits++) {
        if (!write_byte(output, nbits == 1 ? 1 : 0)) {
            return false;
        }
    }
    if (!write_byte(output, 0) || !write_marker(output, 0xda) || !write_word(output, 12) || !write_byte(output, 3)) {
        return false;
    }
    for (component = 1; component <= 3; component++) {
        if (!write_byte(output, (unsigned)component) || !write_byte(output, 0)) {
            return false;
        }
    }
    return write_byte(output, 0) && write_byte(output, 63) && write_byte(output, 0);
}

static void calculate_averages(const unsigned char *pixels, const numeric_vector_t *size, const numeric_vector_t *pos, int *avgs) {
    unsigned totals[3] = {0, 0, 0};
    int npixels = 0;
    int y;
    int x;
    const unsigned char *pixel;

    for (y = pos->y; y < pos->y + 8 && y < size->y; y++) {
        for (x = pos->x; x < pos->x + 8 && x < size->x; x++) {
            pixel = pixels + ((size_t)y * size->x + x) * 3;
            totals[0] += (unsigned)((77 * pixel[0] + 150 * pixel[1] + 29 * pixel[2]) >> 8);
            totals[1] += (unsigned)(((-43 * pixel[0] - 85 * pixel[1] + 128 * pixel[2]) >> 8) + 128);
            totals[2] += (unsigned)(((128 * pixel[0] - 107 * pixel[1] - 21 * pixel[2]) >> 8) + 128);
            npixels++;
        }
    }
    avgs[0] = (int)(totals[0] / (unsigned)npixels) - 128;
    avgs[1] = (int)(totals[1] / (unsigned)npixels) - 128;
    avgs[2] = (int)(totals[2] / (unsigned)npixels) - 128;
}

bool jpeg_encode(const unsigned char *pixels, const numeric_vector_t *size, unsigned char **image, size_t *nbytes) {
    writer_t output = {NULL, 0, 0, 0, 0};
    int prevavgs[3] = {0, 0, 0};
    int y;
    int x;
    int component;
    int avgs[3];
    numeric_vector_t blockpos;

    if (pixels == NULL || size == NULL || image == NULL || nbytes == NULL || size->x <= 0 || size->y <= 0 || 65535 < size->x || 65535 < size->y || !write_headers(&output, size)) {
        free(output.data);
        return false;
    }
    for (y = 0; y < size->y; y += 8) {
        for (x = 0; x < size->x; x += 8) {
            blockpos.x = x;
            blockpos.y = y;
            blockpos.z = 0;
            calculate_averages(pixels, size, &blockpos, avgs);
            for (component = 0; component < 3; component++) {
                if (!write_block(&output, avgs[component], &prevavgs[component])) {
                    free(output.data);
                    return false;
                }
            }
        }
    }
    while (output.nbits != 0) {
        if (!write_bits(&output, 1, 1)) {
            free(output.data);
            return false;
        }
    }
    if (!write_marker(&output, 0xd9)) {
        free(output.data);
        return false;
    }
    *image = output.data;
    *nbytes = output.nbytes;
    return true;
}

