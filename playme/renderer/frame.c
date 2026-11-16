#include <stddef.h>
#include <stdlib.h>

#include "internal.h"
#include "lib.h"

static const int distancecount = 2049;
static const int cloudcosinecount = 256;
static const int skylimit = 1027;

typedef struct {
    short x;
    short y;
    short z;
} raydirection_t;

typedef struct {
    unsigned char *pixels;
    numeric_vector_t size;
    size_t pixelcap;
} framebufferstate_t;

typedef struct {
    void *storage;
    numeric_vector_t size;
    world_mesh_t world;
} raycachestate_t;

static framebufferstate_t *framebuffer_state(renderer_framebuffer_t *framebuffer) {
    return *(framebufferstate_t **)framebuffer->storage;
}

static const framebufferstate_t *constant_framebuffer_state(const renderer_framebuffer_t *framebuffer) {
    return *(framebufferstate_t *const *)framebuffer->storage;
}

static raycachestate_t *cache_state(renderer_raycache_t *cache) {
    return *(raycachestate_t **)cache->storage;
}

static const raycachestate_t *constant_cache_state(const renderer_raycache_t *cache) {
    return *(raycachestate_t *const *)cache->storage;
}

static void set_framebuffer_state(renderer_framebuffer_t *framebuffer, framebufferstate_t *state) {
    *(framebufferstate_t **)framebuffer->storage = state;
}

static void set_cache_state(renderer_raycache_t *cache, raycachestate_t *state) {
    *(raycachestate_t **)cache->storage = state;
}

static const int *Distances(const renderer_raycache_t *cache) {
    return (const int *)constant_cache_state(cache)->storage;
}

static const world_mesh_t *World(const renderer_raycache_t *cache) {
    return &constant_cache_state(cache)->world;
}

static const raydirection_t *RayDirections(const renderer_raycache_t *cache) {
    const void *dirs = (const int *)constant_cache_state(cache)->storage + distancecount;

    return (const raydirection_t *)dirs;
}

static unsigned short *DepthBuffer(renderer_raycache_t *cache) {
    raycachestate_t *state = cache_state(cache);
    size_t ndirs = (size_t)state->size.x * (state->size.y / 2 + 1);
    void *dirs = (int *)state->storage + distancecount;
    void *depth = (raydirection_t *)dirs + ndirs;

    return (unsigned short *)depth;
}

static int ClampedValue(int number, int low, int high) {
    if (number < low) {
        return low;
    }
    return high < number ? high : number;
}

static int ScaledDifference(int diff, numeric_fixed_t amount) {
    return numeric_RawScale(diff, amount.value);
}

static int PixelInterpolation(int a, int b, numeric_fixed_t amount) {
    return a + ScaledDifference(b - a, amount);
}

static int DarkenedPixel(int pixel, numeric_fixed_t amount) {
    return pixel - ((pixel * amount.value) >> 10);
}

static int IntegerValue(int raw) {
    if (raw < 0) {
        raw += 1023;
    }
    return raw >> 10;
}

static bool transform_sky_direction(const numeric_matrix_t *matrix, int *x, int *y, int *z) {
    int x0 = *x;
    int y0 = *y;
    int z0 = *z;

    *y = numeric_RawScale(matrix->yx.value, x0) + numeric_RawScale(matrix->yy.value, y0) + numeric_RawScale(matrix->yz.value, z0);
    if (*y <= 0) {
        return false;
    }
    *x = numeric_RawScale(matrix->xx.value, x0) + numeric_RawScale(matrix->xy.value, y0) + numeric_RawScale(matrix->xz.value, z0);
    *z = numeric_RawScale(matrix->zx.value, x0) + numeric_RawScale(matrix->zy.value, y0) + numeric_RawScale(matrix->zz.value, z0);
    return true;
}

void renderer_draw_moves(renderer_framebuffer_t *framebuffer, unsigned long nmoves) {
    framebufferstate_t *state;
    static const char label[] = "MOVES ";
    char content[27];
    char digits[20];
    int ndigits = 0;
    int labelchar;
    int nchars = 0;
    numeric_vector_t pos = {8, 8, 0, 0};

    if (framebuffer == NULL) {
        return;
    }
    state = framebuffer_state(framebuffer);
    if (state == NULL) {
        return;
    }
    for (labelchar = 0; label[labelchar]; labelchar++) {
        content[nchars++] = label[labelchar];
    }
    do {
        digits[ndigits++] = (char)('0' + nmoves % 10);
        nmoves /= 10;
    } while (nmoves != 0);
    while (0 < ndigits) {
        content[nchars++] = digits[--ndigits];
    }
    content[nchars] = '\0';
    renderer_text_draw(state->pixels, &state->size, &pos, content, 2, 255, 255, 255);
}

static void draw_triangle(renderer_framebuffer_t *framebuffer, unsigned short *depth, const renderer_world_vertex_t *positions, world_material_t surface) {
    framebufferstate_t *state = framebuffer_state(framebuffer);

    renderer_rasterizer_world_triangle(state->pixels, depth, &state->size, positions, surface);
}

static void store_direction(raydirection_t *dirs, int width, int y, int x, int dx, int dy, int dz) {
    int mirrorx;
    raydirection_t *row = dirs + y * width;

    row[x].x = (short)dx;
    row[x].y = (short)dy;
    row[x].z = (short)dz;
    if (x != 0 && x != width - x) {
        mirrorx = width - x;
        row[mirrorx].x = (short)-dx;
        row[mirrorx].y = (short)dy;
        row[mirrorx].z = (short)dz;
    }
}

static bool prepare(renderer_raycache_t *cache, const numeric_vector_t *size) {
    raycachestate_t *state;
    numeric_vector_t screenoffset;
    numeric_fixedvector_t dir;
    void *storage;
    void *dirstorage;
    raydirection_t *dirs;
    int *distances;
    size_t npixels;
    size_t ndirs;
    size_t nbytes;
    int dirwidth;
    int dirheight;
    numeric_fixed_t diry;
    int squarestart;
    int x;
    int y;

    if (size->x <= 0 || size->y <= 0) {
        return false;
    }
    state = cache_state(cache);
    if (state != NULL && state->storage != NULL && state->size.x == size->x && state->size.y == size->y) {
        return true;
    }
    if (((size_t)-1) / (size_t)size->x < (size_t)size->y) {
        return false;
    }
    npixels = (size_t)size->x * size->y;
    dirwidth = size->x;
    dirheight = size->y / 2 + 1;
    if (((size_t)-1) / (size_t)dirwidth < (size_t)dirheight) {
        return false;
    }
    ndirs = (size_t)dirwidth * dirheight;
    if (((size_t)-1) / sizeof(raydirection_t) < ndirs || ((size_t)-1) / sizeof(unsigned short) < npixels) {
        return false;
    }
    nbytes = distancecount * sizeof(int);
    if (((size_t)-1) - nbytes < ndirs * sizeof(raydirection_t)) {
        return false;
    }
    nbytes += ndirs * sizeof(raydirection_t);
    if (((size_t)-1) - nbytes < npixels * sizeof(unsigned short)) {
        return false;
    }
    nbytes += npixels * sizeof(unsigned short);
    storage = malloc(nbytes);
    if (storage == NULL) {
        return false;
    }
    distances = (int *)storage;
    dirstorage = distances + distancecount;
    dirs = (raydirection_t *)dirstorage;
    if (size->y <= size->x && ((size->x - size->y) & 1) == 0) {
        squarestart = (size->x - size->y) / 2;
        for (y = 0; y < dirheight; y++) {
            screenoffset.y = size->y - (y << 1);
            diry = numeric_Ratio(screenoffset.y, size->y);
            for (x = 0; x < squarestart; x++) {
                screenoffset.x = size->x - (x << 1);
                dir.x = numeric_Ratio(-screenoffset.x, size->y);
                dir.y = diry;
                dir.z.value = -1024;
                dir.w.value = 0;
                numeric_normalize(&dir);
                store_direction(dirs, dirwidth, y, x, numeric_RawValue(dir.x), numeric_RawValue(dir.y), numeric_RawValue(dir.z));
            }
            for (x = 0; x <= y; x++) {
                screenoffset.x = size->y - (x << 1);
                dir.x = numeric_Ratio(-screenoffset.x, size->y);
                dir.y = diry;
                dir.z.value = -1024;
                dir.w.value = 0;
                numeric_normalize(&dir);
                store_direction(dirs, dirwidth, y, squarestart + x, numeric_RawValue(dir.x), numeric_RawValue(dir.y), numeric_RawValue(dir.z));
                if (x != y) {
                    store_direction(dirs, dirwidth, x, squarestart + y, -numeric_RawValue(dir.y), -numeric_RawValue(dir.x), numeric_RawValue(dir.z));
                }
            }
        }
    } else {
        for (y = 0; y < dirheight; y++) {
            screenoffset.y = size->y - (y << 1);
            diry = numeric_Ratio(screenoffset.y, size->y);
            for (x = 0; x <= dirwidth / 2; x++) {
                screenoffset.x = size->x - (x << 1);
                dir.x = numeric_Ratio(-screenoffset.x, size->y);
                dir.y = diry;
                dir.z.value = -1024;
                dir.w.value = 0;
                numeric_normalize(&dir);
                store_direction(dirs, dirwidth, y, x, numeric_RawValue(dir.x), numeric_RawValue(dir.y), numeric_RawValue(dir.z));
            }
        }
    }
    for (y = 1; y < distancecount; y++) {
        distances[y] = 819200 / y;
    }
    if (state == NULL) {
        state = (raycachestate_t *)calloc(1, sizeof(raycachestate_t));
        if (state == NULL) {
            free(storage);
            return false;
        }
        state->storage = NULL;
        if (!world_build(&state->world)) {
            free(state);
            free(storage);
            return false;
        }
        set_cache_state(cache, state);
    }
    free(state->storage);
    state->storage = storage;
    state->size = *size;
    return true;
}

static void draw_background(renderer_framebuffer_t *framebuffer, const unsigned short *depth, const renderer_raycache_t *cache, const numeric_fixedvector_t *camerapos, const numeric_quaternion_t *camerarot,
                            const numeric_fixedvector_t *sun) {
    framebufferstate_t *state = framebuffer_state(framebuffer);
    const raydirection_t *dirs = RayDirections(cache);
    const int *distances = Distances(cache);
    numeric_matrix_t skyrot;
    numeric_fixedvector_t sundir = *sun;
    bool yawonly;
    int darkness;
    numeric_fixed_t darknessamount;
    numeric_fixed_t quarterdarkness;
    unsigned skycolors[2055];
    int skyy;
    int red;
    int green;
    int blue;
    int blueshade;
    short cloudquarters[256];
    short cloudtwelfths[256];
    const unsigned short *depthpixel;
    unsigned char *pixel;
    int y;
    int sourcey;
    bool ymirrored;
    int ymask;
    const raydirection_t *dirrow;
    int x;
    int dirx;
    int diry;
    int dirz;
    bool upward;
    unsigned skycolor;
    int distance;
    int hitx;
    int hitz;
    int cloud;
    int cloudcolor;
    int visibility;
    numeric_fixed_t visibilityamount;

    numeric_set_quaternion(&skyrot, *camerarot);
    yawonly = skyrot.xy.value == 0 && skyrot.yx.value == 0 && skyrot.yz.value == 0 && skyrot.zy.value == 0 && skyrot.yy.value == 1024;
    numeric_normalize(&sundir);
    darkness = ClampedValue(-sundir.y.value * 2, 0, 1024);
    darknessamount = numeric_FixedFromRaw(darkness);
    quarterdarkness = numeric_FixedFromRaw(darkness / 4);
    for (skyy = -skylimit; skyy <= skylimit; skyy++) {
        red = 128 - numeric_RawScale(48, skyy);
        green = 179 - numeric_RawScale(60, skyy);
        blueshade = numeric_RawScale(20, skyy);
        if (blueshade < 0) {
            blueshade = 0;
        }
        blue = 255 - blueshade;
        red = DarkenedPixel(red, darknessamount);
        green = DarkenedPixel(green, darknessamount);
        blue = DarkenedPixel(blue, quarterdarkness);
        skycolors[skyy + skylimit] = (unsigned)red | (unsigned)green << 8 | (unsigned)blue << 16;
    }
    for (skyy = 0; skyy < cloudcosinecount; skyy++) {
        cloudquarters[skyy] = (short)(numeric_RawCosine(skyy) / 4);
        cloudtwelfths[skyy] = (short)(cloudquarters[skyy] / 3);
    }
    depthpixel = depth;
    pixel = state->pixels;
    for (y = 0; y < state->size.y; y++) {
        sourcey = y <= state->size.y / 2 ? y : state->size.y - y;
        ymirrored = state->size.y / 2 < y;
        ymask = -ymirrored;
        dirrow = dirs + sourcey * state->size.x;
        for (x = 0; x < state->size.x; x++, depthpixel++, pixel += 3) {
            if (*depthpixel != 65535) {
                continue;
            }
            dirx = dirrow[x].x;
            diry = dirrow[x].y;
            dirz = dirrow[x].z;
            diry = (diry ^ ymask) - ymask;
            if (yawonly) {
                upward = 0 < diry;
                if (upward) {
                    int x0 = dirx;
                    int z0 = dirz;

                    dirx = numeric_RawScale(skyrot.xx.value, x0) + numeric_RawScale(skyrot.xz.value, z0);
                    dirz = numeric_RawScale(skyrot.zx.value, x0) + numeric_RawScale(skyrot.zz.value, z0);
                }
            } else {
                upward = transform_sky_direction(&skyrot, &dirx, &diry, &dirz);
            }
            if (-skylimit <= diry && diry <= skylimit) {
                skycolor = skycolors[diry + skylimit];
                pixel[0] = (unsigned char)skycolor;
                pixel[1] = (unsigned char)(skycolor >> 8);
                pixel[2] = (unsigned char)(skycolor >> 16);
            } else {
                red = 128 - numeric_RawScale(48, diry);
                green = 179 - numeric_RawScale(60, diry);
                blueshade = numeric_RawScale(20, diry);
                if (blueshade < 0) {
                    blueshade = 0;
                }
                blue = 255 - blueshade;
                pixel[0] = (unsigned char)DarkenedPixel(red, darknessamount);
                pixel[1] = (unsigned char)DarkenedPixel(green, darknessamount);
                pixel[2] = (unsigned char)DarkenedPixel(blue, quarterdarkness);
            }
            if (!upward) {
                continue;
            }
            if (diry < distancecount) {
                distance = distances[diry];
            } else {
                distance = 819200 / diry;
            }
            hitx = IntegerValue(camerapos->x.value + dirx * distance);
            hitz = IntegerValue(camerapos->z.value + dirz * distance);
            cloud = cloudquarters[(cloudquarters[(hitz / 32) & (cloudcosinecount - 1)] + hitx / 8) & (cloudcosinecount - 1)];
            cloud += cloudtwelfths[(hitz * 16 / 75) & (cloudcosinecount - 1)] - 10;
            if (0 <= cloud) {
                continue;
            }
            cloudcolor = ClampedValue(256 + cloud / 2, 128, 255);
            visibility = ClampedValue((diry - 64) * 8, 0, 1024);
            visibilityamount.value = visibility;
            pixel[0] = (unsigned char)PixelInterpolation(pixel[0], cloudcolor, visibilityamount);
            pixel[1] = (unsigned char)PixelInterpolation(pixel[1], cloudcolor, visibilityamount);
            pixel[2] = (unsigned char)PixelInterpolation(pixel[2], cloudcolor, visibilityamount);
        }
    }
}

static void draw_map(renderer_framebuffer_t *framebuffer, renderer_raycache_t *cache, const numeric_fixedvector_t *camerapos, const numeric_quaternion_t *camerarot, const numeric_fixedvector_t *sun) {
    framebufferstate_t *state = framebuffer_state(framebuffer);
    unsigned short *depth = DepthBuffer(cache);
    const world_mesh_t *mesh = World(cache);
    numeric_matrix_t viewrotation;
    numeric_matrix_t projection;
    renderer_world_vertex_t trivertices[3];
    renderer_world_vertex_t vertices[sizeof(mesh->positions) / sizeof(mesh->positions[0])];
    numeric_fixedvector_t worldpos;
    int npositions = (int)(sizeof(mesh->positions) / sizeof(mesh->positions[0]));
    int ntriangles = (int)(sizeof(mesh->triangles) / sizeof(mesh->triangles[0]));
    int tri;
    int pos;
    int vertex;
    numeric_quaternion_t inverse;

    renderer_rasterizer_clear(depth, &state->size);
    inverse.x = numeric_Negative(camerarot->x);
    inverse.y = numeric_Negative(camerarot->y);
    inverse.z = numeric_Negative(camerarot->z);
    inverse.w = camerarot->w;
    numeric_set_quaternion(&viewrotation, inverse);
    numeric_set_perspective(&projection, state->size.x, state->size.y, 1, 65535);
    for (pos = 0; pos < npositions; pos++) {
        worldpos = numeric_Point(mesh->positions[pos].x, mesh->positions[pos].y, mesh->positions[pos].z);
        renderer_rasterizer_transform_vertex(&worldpos, camerapos, &viewrotation, &projection, &vertices[pos]);
    }
    for (tri = 0; tri < ntriangles; tri++) {
        for (vertex = 0; vertex < 3; vertex++) {
            trivertices[vertex] = vertices[mesh->triangles[tri].vertices[vertex]];
        }
        draw_triangle(framebuffer, depth, trivertices, mesh->triangles[tri].surface);
    }
    draw_background(framebuffer, depth, cache, camerapos, camerarot, sun);
}

bool renderer_create_cached(renderer_raycache_t *cache, const numeric_fixedvector_t *camerapos, const numeric_quaternion_t *camerarot, const numeric_fixedvector_t *sun, const numeric_vector_t *size,
                          renderer_framebuffer_t *framebuffer) {
    framebufferstate_t *state;
    unsigned char *pixels;
    size_t nbytes;

    if (cache == NULL || camerapos == NULL || camerarot == NULL || sun == NULL || size == NULL || framebuffer == NULL) {
        return false;
    }
    if (size->x <= 0 || size->y <= 0) {
        return false;
    }
    if (!prepare(cache, size)) {
        return false;
    }
    if (((size_t)-1) / (size_t)size->x < (size_t)size->y) {
        return false;
    }
    nbytes = (size_t)size->x * size->y;
    if (((size_t)-1) / 3 < nbytes) {
        return false;
    }
    nbytes *= 3;
    state = framebuffer_state(framebuffer);
    if (state == NULL) {
        state = (framebufferstate_t *)malloc(sizeof(framebufferstate_t));
        if (state == NULL) {
            return false;
        }
        state->pixels = NULL;
        state->size.x = 0;
        state->size.y = 0;
        state->size.z = 0;
        state->size.w = 0;
        state->pixelcap = 0;
        set_framebuffer_state(framebuffer, state);
    }
    pixels = state->pixels;
    if (pixels == NULL || state->pixelcap < nbytes) {
        pixels = (unsigned char *)realloc(state->pixels, nbytes);
        if (pixels == NULL) {
            return false;
        }
        state->pixels = pixels;
        state->pixelcap = nbytes;
    }
    state->size = *size;
    draw_map(framebuffer, cache, camerapos, camerarot, sun);
    return true;
}

static void release_cache(renderer_raycache_t *cache) {
    raycachestate_t *state;

    if (cache == NULL) {
        return;
    }
    state = cache_state(cache);
    if (state == NULL) {
        return;
    }
    free(state->storage);
    free(state);
    set_cache_state(cache, NULL);
}

bool renderer_create(const numeric_fixedvector_t *camerapos, const numeric_quaternion_t *camerarot, const numeric_fixedvector_t *sun, const numeric_vector_t *size, renderer_framebuffer_t *framebuffer) {
    renderer_raycache_t cache = {{0}};
    bool created;

    created = renderer_create_cached(&cache, camerapos, camerarot, sun, size, framebuffer);
    release_cache(&cache);
    return created;
}

void renderer_destroy_cache(renderer_raycache_t *cache) {
    release_cache(cache);
}

void renderer_destroy_framebuffer(renderer_framebuffer_t *framebuffer) {
    framebufferstate_t *state;

    if (framebuffer == NULL) {
        return;
    }
    state = framebuffer_state(framebuffer);
    if (state == NULL) {
        return;
    }
    free(state->pixels);
    free(state);
    set_framebuffer_state(framebuffer, NULL);
}

const unsigned char *renderer_Pixels(const renderer_framebuffer_t *framebuffer) {
    const framebufferstate_t *state;

    if (framebuffer == NULL) {
        return NULL;
    }
    state = constant_framebuffer_state(framebuffer);
    return state == NULL ? NULL : state->pixels;
}

numeric_vector_t renderer_FramebufferSize(const renderer_framebuffer_t *framebuffer) {
    const framebufferstate_t *state;
    numeric_vector_t size = {0, 0, 0, 0};

    if (framebuffer == NULL) {
        return size;
    }
    state = constant_framebuffer_state(framebuffer);
    return state == NULL ? size : state->size;
}

size_t renderer_PixelCapacity(const renderer_framebuffer_t *framebuffer) {
    const framebufferstate_t *state;

    if (framebuffer == NULL) {
        return 0;
    }
    state = constant_framebuffer_state(framebuffer);
    return state == NULL ? 0 : state->pixelcap;
}

bool renderer_CacheEmpty(const renderer_raycache_t *cache) {
    return cache == NULL || constant_cache_state(cache) == NULL;
}
