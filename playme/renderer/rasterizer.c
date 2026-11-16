#include <stddef.h>

#include "internal.h"

#if defined(_WIN32)
typedef __int64 wide_t;
typedef unsigned __int64 uwide_t;
#else
typedef long wide_t;
typedef unsigned long uwide_t;
#endif

static const wide_t gradientscale = 524288;
static const wide_t perspectivescale = 4194304;

static const signed char grasswave[256] = {
    0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  4,  5,  5,  5,  5,
    5,  5,  5,  6,  6,  6,  6,  6,  6,  6,  6,  6,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
    8,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  6,  6,  6,  6,  6,  6,  6,  6,  6,  5,  5,
    5,  5,  5,  5,  5,  4,  4,  4,  4,  4,  4,  3,  3,  3,  3,  3,  3,  2,  2,  2,  2,  2,  1,  1,  1,  1,  1,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  -1, -1, -1, -1, -1, -2, -2, -2, -2, -2, -3, -3, -3, -3, -3, -3, -4, -4, -4, -4, -4, -4, -5, -5, -5, -5,
    -5, -5, -5, -6, -6, -6, -6, -6, -6, -6, -6, -6, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7,
    -8, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -7, -6, -6, -6, -6, -6, -6, -6, -6, -6, -5, -5,
    -5, -5, -5, -5, -5, -4, -4, -4, -4, -4, -4, -3, -3, -3, -3, -3, -3, -2, -2, -2, -2, -2, -1, -1, -1, -1, -1, 0,  0,  0,  0,  0};

static const signed char grasshalf[27] = {-4, -3, -3, -2, -2, -1, -1, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9};

static const unsigned char grasschecker[32] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10};

typedef enum {
    RASTERIZER_PLANE_LEFT,
    RASTERIZER_PLANE_RIGHT,
    RASTERIZER_PLANE_BOTTOM,
    RASTERIZER_PLANE_TOP,
    RASTERIZER_PLANE_NEAR,
    RASTERIZER_PLANE_FAR,
    RASTERIZER_PLANE_COUNT
} plane_t;

typedef struct {
    int x;
    int y;
    wide_t invdepth;
    wide_t worldxdepth;
    wide_t worldydepth;
    wide_t worldzdepth;
} screenvertex_t;

typedef struct {
    renderer_world_vertex_t vertices[10];
} polygon_t;

typedef struct {
    int red;
    int green;
    int blue;
} color_t;

static wide_t Quotient(wide_t numerator, wide_t denominator) {
    wide_t quotient = (wide_t)((double)numerator / (double)denominator);
    wide_t numeratorrem = numerator - quotient * denominator;

    if (0 <= numerator) {
        if (numeratorrem < 0) {
            quotient--;
        } else if (denominator <= numeratorrem) {
            quotient++;
        }
    } else {
        if (0 < numeratorrem) {
            quotient++;
        } else if (numeratorrem <= -denominator) {
            quotient--;
        }
    }
    return quotient;
}

static wide_t NonnegativeQuotient(wide_t numerator, wide_t denominator) {
    wide_t quotient = (wide_t)((double)numerator / (double)denominator);
    wide_t numeratorrem = numerator - quotient * denominator;

    if (numeratorrem < 0) {
        quotient--;
    } else if (denominator <= numeratorrem) {
        quotient++;
    }
    return quotient;
}

static wide_t NonpositiveQuotient(wide_t numerator, wide_t denominator) {
    wide_t quotient = (wide_t)((double)numerator / (double)denominator);
    wide_t numeratorrem = numerator - quotient * denominator;

    if (0 < numeratorrem) {
        quotient++;
    } else if (numeratorrem <= -denominator) {
        quotient--;
    }
    return quotient;
}

static int Minimum(int a, int b) {
    return a < b ? a : b;
}

static int Maximum(int a, int b) {
    return b < a ? a : b;
}

static wide_t PlaneDistance(const renderer_world_vertex_t *vertex, plane_t plane) {
    if (plane == RASTERIZER_PLANE_LEFT) {
        return (wide_t)vertex->clippos.w.value + vertex->clippos.x.value;
    }
    if (plane == RASTERIZER_PLANE_RIGHT) {
        return (wide_t)vertex->clippos.w.value - vertex->clippos.x.value;
    }
    if (plane == RASTERIZER_PLANE_BOTTOM) {
        return (wide_t)vertex->clippos.w.value + vertex->clippos.y.value;
    }
    if (plane == RASTERIZER_PLANE_TOP) {
        return (wide_t)vertex->clippos.w.value - vertex->clippos.y.value;
    }
    if (plane == RASTERIZER_PLANE_NEAR) {
        return vertex->clippos.z.value;
    }
    return (wide_t)vertex->clippos.w.value - vertex->clippos.z.value;
}

static unsigned ClipCode(const renderer_world_vertex_t *vertex) {
    unsigned code = 0;
    int plane;

    for (plane = 0; plane < RASTERIZER_PLANE_COUNT; plane++) {
        if (PlaneDistance(vertex, (plane_t)plane) < 0) {
            code |= 1U << plane;
        }
    }
    return code;
}

static wide_t Interpolation(wide_t a, wide_t b, wide_t numerator, wide_t denominator) {
    return a + (b - a) * numerator / denominator;
}

static numeric_fixed_t CoordinateInterpolation(numeric_fixed_t a, numeric_fixed_t b, wide_t numerator, wide_t denominator) {
    return numeric_FixedFromRaw((int)Interpolation(a.value, b.value, numerator, denominator));
}

static renderer_world_vertex_t ClipIntersection(const renderer_world_vertex_t *a, const renderer_world_vertex_t *b, wide_t da, wide_t db) {
    renderer_world_vertex_t intersection;
    wide_t denom = da - db;

    intersection.clippos.x = CoordinateInterpolation(a->clippos.x, b->clippos.x, da, denom);
    intersection.clippos.y = CoordinateInterpolation(a->clippos.y, b->clippos.y, da, denom);
    intersection.clippos.z = CoordinateInterpolation(a->clippos.z, b->clippos.z, da, denom);
    intersection.clippos.w = CoordinateInterpolation(a->clippos.w, b->clippos.w, da, denom);
    intersection.worldpos.x = CoordinateInterpolation(a->worldpos.x, b->worldpos.x, da, denom);
    intersection.worldpos.y = CoordinateInterpolation(a->worldpos.y, b->worldpos.y, da, denom);
    intersection.worldpos.z = CoordinateInterpolation(a->worldpos.z, b->worldpos.z, da, denom);
    intersection.worldpos.w = CoordinateInterpolation(a->worldpos.w, b->worldpos.w, da, denom);
    return intersection;
}

static void set_color(color_t *color, int red, int green, int blue) {
    color->red = red;
    color->green = green;
    color->blue = blue;
}

static bool sample_material(world_material_t surface, int x, int y, int z, int depth, color_t *color) {
    int pattern;
    int fog;
    int cosine;

    if (surface == WORLD_MATERIAL_GRASS) {
        pattern = grasschecker[(x ^ z) & 31] + grasswave[(x + z + 64) & 255];
        set_color(color, 30 + pattern, 112 + pattern, 22 + grasshalf[pattern + 8]);
    } else {
        switch (surface) {
            case WORLD_MATERIAL_GRASS:
                break;
            case WORLD_MATERIAL_WATER:
                cosine = numeric_RawCosine(x * 3 + z * 5);
                pattern = numeric_RawScale(18, cosine);
                set_color(color, 30 + pattern / 3, 105 + pattern, 155 + pattern);
                break;
            case WORLD_MATERIAL_DIRT:
                pattern = (unsigned)((x & 31) - 15) < 3 ? 18 : 0;
                set_color(color, 105 + pattern, 100 + pattern, 85 + pattern);
                break;
            case WORLD_MATERIAL_WALL:
                pattern = (((x >> 3) + (y >> 3) + (z >> 3)) & 1) * 12;
                set_color(color, 174 + pattern, 130 + pattern, 84 + pattern);
                break;
            case WORLD_MATERIAL_ROOF:
                pattern = ((x + z) & 15) < 3 ? 18 : 0;
                set_color(color, 92 + pattern, 31 + pattern / 3, 27 + pattern / 3);
                break;
            default:
                return false;
        }
    }
    fog = depth <= 512 ? 0 : (depth - 512) >> 2;
    if (1024 < fog) {
        fog = 1024;
    }
    if (surface == WORLD_MATERIAL_WALL) {
        color->red += numeric_RawScale(128 - color->red, fog);
    } else {
        color->red += ((128 - color->red) * fog) >> 10;
    }
    color->green += ((179 - color->green) * fog) >> 10;
    color->blue += ((255 - color->blue) * fog) >> 10;
    return true;
}

static void project_clip(const renderer_world_vertex_t *input, const numeric_vector_t *size, screenvertex_t *output) {
    int depth = numeric_IntegerValue(input->clippos.w);

    if (depth < 1) {
        depth = 1;
    }
    output->x = size->x / 2 + (int)((wide_t)input->clippos.x.value * size->x / ((wide_t)input->clippos.w.value * 2));
    output->y = size->y / 2 - (int)((wide_t)input->clippos.y.value * size->y / ((wide_t)input->clippos.w.value * 2));
    output->invdepth = perspectivescale / depth;
    output->worldxdepth = (wide_t)input->worldpos.x.value * output->invdepth;
    output->worldydepth = (wide_t)input->worldpos.y.value * output->invdepth;
    output->worldzdepth = (wide_t)input->worldpos.z.value * output->invdepth;
}

void renderer_rasterizer_clear(unsigned short *depth, const numeric_vector_t *size) {
    size_t pixel;

    for (pixel = 0; pixel < (size_t)size->x * size->y; pixel++) {
        depth[pixel] = 65535;
    }
}

static void rasterize(unsigned char *pixels, unsigned short *depth, const numeric_vector_t *size, const screenvertex_t *a, const screenvertex_t *b, const screenvertex_t *c, world_material_t surface) {
    wide_t ix = b->x - a->x;
    wide_t iy = b->y - a->y;
    wide_t jx = c->x - a->x;
    wide_t jy = c->y - a->y;
    wide_t det = ix * jy - iy * jx;
    wide_t ci;
    wide_t cj;
    wide_t cir;
    wide_t cjr;
    wide_t pi0;
    wide_t pj0;
    wide_t ci0;
    wide_t cj0;
    wide_t pix;
    wide_t pjx;
    wide_t piy;
    wide_t pjy;
    wide_t inv;
    wide_t wx;
    wide_t wy;
    wide_t wz;
    wide_t inv0;
    wide_t wx0;
    wide_t wy0;
    wide_t wz0;
    wide_t invx;
    wide_t wxx;
    wide_t wyx;
    wide_t wzx;
    wide_t invy;
    wide_t wxy;
    wide_t wyy;
    wide_t wzy;
    color_t color;
    int xl;
    int yl;
    int xh;
    int yh;
    int x;
    int xend;
    int xoff;
    int y;
    int pixeldepth;
    int worldx;
    int worldy;
    int worldz;
    unsigned short *depthpixel;
    unsigned char *pixel;
    const screenvertex_t *v;

    if (det == 0) {
        return;
    }
    if (det < 0) {
        v = b;
        b = c;
        c = v;
        ix = b->x - a->x;
        iy = b->y - a->y;
        jx = c->x - a->x;
        jy = c->y - a->y;
        det = -det;
    }
    xl = Maximum(0, Minimum(a->x, Minimum(b->x, c->x)));
    yl = Maximum(0, Minimum(a->y, Minimum(b->y, c->y)));
    xh = Minimum(size->x - 1, Maximum(a->x, Maximum(b->x, c->x)));
    yh = Minimum(size->y - 1, Maximum(a->y, Maximum(b->y, c->y)));
    if (xh < xl || yh < yl) {
        return;
    }
    pi0 = ((wide_t)(xl - a->x) * jy - (wide_t)(yl - a->y) * jx) * gradientscale / det;
    pj0 = (ix * (yl - a->y) - iy * (xl - a->x)) * gradientscale / det;
    ci0 = (wide_t)(xl - a->x) * jy - (wide_t)(yl - a->y) * jx;
    cj0 = ix * (yl - a->y) - iy * (xl - a->x);
    pix = jy * gradientscale / det;
    pjx = -iy * gradientscale / det;
    piy = -jx * gradientscale / det;
    pjy = ix * gradientscale / det;
    inv0 = a->invdepth * gradientscale + pi0 * (b->invdepth - a->invdepth) + pj0 * (c->invdepth - a->invdepth);
    wx0 = a->worldxdepth * gradientscale + pi0 * (b->worldxdepth - a->worldxdepth) + pj0 * (c->worldxdepth - a->worldxdepth);
    wy0 = a->worldydepth * gradientscale + pi0 * (b->worldydepth - a->worldydepth) + pj0 * (c->worldydepth - a->worldydepth);
    wz0 = a->worldzdepth * gradientscale + pi0 * (b->worldzdepth - a->worldzdepth) + pj0 * (c->worldzdepth - a->worldzdepth);
    invx = pix * (b->invdepth - a->invdepth) + pjx * (c->invdepth - a->invdepth);
    wxx = pix * (b->worldxdepth - a->worldxdepth) + pjx * (c->worldxdepth - a->worldxdepth);
    wyx = pix * (b->worldydepth - a->worldydepth) + pjx * (c->worldydepth - a->worldydepth);
    wzx = pix * (b->worldzdepth - a->worldzdepth) + pjx * (c->worldzdepth - a->worldzdepth);
    invy = piy * (b->invdepth - a->invdepth) + pjy * (c->invdepth - a->invdepth);
    wxy = piy * (b->worldxdepth - a->worldxdepth) + pjy * (c->worldxdepth - a->worldxdepth);
    wyy = piy * (b->worldydepth - a->worldydepth) + pjy * (c->worldydepth - a->worldydepth);
    wzy = piy * (b->worldzdepth - a->worldzdepth) + pjy * (c->worldzdepth - a->worldzdepth);
    for (y = yl; y <= yh; y++) {
        ci = ci0;
        cj = cj0;
        x = xl;
        while (x <= xh && !((uwide_t)ci <= (uwide_t)det && (uwide_t)cj <= (uwide_t)(det - ci))) {
            ci += jy;
            cj -= iy;
            x++;
        }
        xoff = x - xl;
        inv = inv0 + xoff * invx;
        wx = wx0 + xoff * wxx;
        wy = wy0 + xoff * wyx;
        wz = wz0 + xoff * wzx;
        depthpixel = depth + y * size->x + x;
        pixel = pixels + (y * size->x + x) * 3;
        xend = xh;
        cir = ci0 + (wide_t)(xh - xl) * jy;
        cjr = cj0 - (wide_t)(xh - xl) * iy;
        while (x <= xend && !((uwide_t)cir <= (uwide_t)det && (uwide_t)cjr <= (uwide_t)(det - cir))) {
            cir -= jy;
            cjr += iy;
            xend--;
        }
        for (; x <= xend; x++, depthpixel++, pixel += 3) {
            if (0 < inv && perspectivescale * gradientscale < (wide_t)*depthpixel * inv) {
                pixeldepth = (int)NonnegativeQuotient(perspectivescale * gradientscale, inv);
                worldx = (int)Quotient(wx, inv * 1024);
                worldy = surface == WORLD_MATERIAL_WALL ? (int)NonnegativeQuotient(wy, inv * 1024) : 0;
                worldz = surface == WORLD_MATERIAL_DIRT ? 0 : (int)NonpositiveQuotient(wz, inv * 1024);
                if (sample_material(surface, worldx, worldy, worldz, pixeldepth, &color)) {
                    *depthpixel = (unsigned short)pixeldepth;
                    pixel[0] = (unsigned char)color.red;
                    pixel[1] = (unsigned char)color.green;
                    pixel[2] = (unsigned char)color.blue;
                }
            }
            inv += invx;
            wx += wxx;
            wy += wyx;
            wz += wzx;
        }
        ci0 -= jx;
        cj0 += ix;
        inv0 += invy;
        wx0 += wxy;
        wy0 += wyy;
        wz0 += wzy;
    }
}

void renderer_rasterizer_transform_vertex(const numeric_fixedvector_t *pos, const numeric_fixedvector_t *camerapos, const numeric_matrix_t *viewrot, const numeric_matrix_t *projection,
                             renderer_world_vertex_t *output) {
    output->clippos = *pos;
    output->clippos.x = numeric_Difference(pos->x, camerapos->x);
    output->clippos.y = numeric_Difference(pos->y, camerapos->y);
    output->clippos.z = numeric_Difference(pos->z, camerapos->z);
    numeric_transform(viewrot, &output->clippos);
    numeric_transform(projection, &output->clippos);
    output->worldpos = *pos;
}

void renderer_rasterizer_world_triangle(unsigned char *pixels, unsigned short *depth, const numeric_vector_t *size, const renderer_world_vertex_t *positions, world_material_t surface) {
    polygon_t input;
    polygon_t output;
    screenvertex_t vertices[10];
    wide_t curdist;
    wide_t nextdist;
    unsigned codes = 0;
    unsigned commoncode = ~0U;
    unsigned code;
    int plane;
    int vertex;
    int ninput = 3;
    int noutput;

    for (vertex = 0; vertex < 3; vertex++) {
        input.vertices[vertex] = positions[vertex];
        code = ClipCode(&input.vertices[vertex]);
        codes |= code;
        commoncode &= code;
    }
    if (commoncode != 0) {
        return;
    }
    for (plane = 0; codes != 0 && plane < RASTERIZER_PLANE_COUNT; plane++) {
        noutput = 0;
        for (vertex = 0; vertex < ninput; vertex++) {
            curdist = PlaneDistance(&input.vertices[vertex], (plane_t)plane);
            nextdist = PlaneDistance(&input.vertices[(vertex + 1) % ninput], (plane_t)plane);
            if (0 <= curdist && 0 <= nextdist) {
                output.vertices[noutput++] = input.vertices[(vertex + 1) % ninput];
            } else if (0 <= curdist) {
                output.vertices[noutput++] = ClipIntersection(&input.vertices[vertex], &input.vertices[(vertex + 1) % ninput], curdist, nextdist);
            } else if (0 <= nextdist) {
                output.vertices[noutput++] = ClipIntersection(&input.vertices[vertex], &input.vertices[(vertex + 1) % ninput], curdist, nextdist);
                output.vertices[noutput++] = input.vertices[(vertex + 1) % ninput];
            }
        }
        if (noutput < 3) {
            return;
        }
        input = output;
        ninput = noutput;
    }
    for (vertex = 0; vertex < ninput; vertex++) {
        project_clip(&input.vertices[vertex], size, &vertices[vertex]);
    }
    for (vertex = 1; vertex + 1 < ninput; vertex++) {
        rasterize(pixels, depth, size, &vertices[0], &vertices[vertex], &vertices[vertex + 1], surface);
    }
}
