#ifndef PLAYME_RENDERER_INTERNAL_H
#define PLAYME_RENDERER_INTERNAL_H

#include "../numeric.h"
#include "world.h"

typedef struct {
    numeric_fixedvector_t clippos;
    numeric_fixedvector_t worldpos;
} renderer_world_vertex_t;

void renderer_rasterizer_clear(unsigned short *depth, const numeric_vector_t *size);
void renderer_rasterizer_transform_vertex(const numeric_fixedvector_t *pos, const numeric_fixedvector_t *camerapos, const numeric_matrix_t *viewrot, const numeric_matrix_t *projection,
                                          renderer_world_vertex_t *output);
void renderer_rasterizer_world_triangle(unsigned char *pixels, unsigned short *depth, const numeric_vector_t *size, const renderer_world_vertex_t *positions, world_material_t surface);
void renderer_text_draw(unsigned char *pixels, const numeric_vector_t *size, const numeric_vector_t *pos, const char *content, int scale, int red, int green, int blue);

#endif
