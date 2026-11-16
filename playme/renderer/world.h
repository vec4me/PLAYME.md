#ifndef PLAYME_RENDERER_WORLD_H
#define PLAYME_RENDERER_WORLD_H

#if defined(__STDC_VERSION__) && __STDC_VERSION__ < 199901L
    #define bool int
    #define true 1
    #define false 0
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ < 202311L
    #include <stdbool.h>
#elif defined(__STDC_VERSION__)
#elif defined(__cplusplus)
#else
    #define bool int
    #define true 1
    #define false 0
#endif

typedef enum {
    WORLD_MATERIAL_GRASS,
    WORLD_MATERIAL_WATER,
    WORLD_MATERIAL_DIRT,
    WORLD_MATERIAL_WALL,
    WORLD_MATERIAL_ROOF
} world_material_t;

typedef struct {
    int x;
    int y;
    int z;
} world_position_t;

typedef struct {
    unsigned short vertices[3];
    world_material_t surface;
} world_triangle_t;

typedef struct {
    world_position_t positions[335];
    world_triangle_t triangles[552];
} world_mesh_t;

#if defined(__cplusplus)
extern "C" {
#endif

bool world_build(world_mesh_t *mesh);

#if defined(__cplusplus)
}
#endif

#endif
