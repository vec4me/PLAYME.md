#include <stddef.h>
#include "world.h"

static const unsigned tablecapacity = 1024;

static unsigned PositionHash(int x, int y, int z) {
    return (unsigned)x * 73856093u ^ (unsigned)y * 19349663u ^ (unsigned)z * 83492791u;
}

static void set_triangle(world_triangle_t *tri, world_material_t surface, int a, int b, int c) {
    tri->surface = surface;
    tri->vertices[0] = (unsigned short)a;
    tri->vertices[1] = (unsigned short)b;
    tri->vertices[2] = (unsigned short)c;
}

static int add_position(world_mesh_t *mesh, int *npositions, int *table, int x, int y, int z) {
    unsigned slot = PositionHash(x, y, z) & (tablecapacity - 1);
    int pos;

    while (0 <= table[slot]) {
        pos = table[slot];
        if (mesh->positions[pos].x == x && mesh->positions[pos].y == y && mesh->positions[pos].z == z) {
            return pos;
        }
        slot = (slot + 1) & (tablecapacity - 1);
    }
    if ((int)(sizeof(mesh->positions) / sizeof(mesh->positions[0])) <= *npositions) {
        return -1;
    }
    pos = (*npositions)++;
    mesh->positions[pos].x = x;
    mesh->positions[pos].y = y;
    mesh->positions[pos].z = z;
    table[slot] = pos;
    return pos;
}

static bool set_quadrilateral(world_mesh_t *mesh, int *npositions, int *table, int *ntriangles, world_material_t surface, int ax, int ay, int az, int bx, int by, int bz, int cx, int cy, int cz, int dx,
                              int dy, int dz) {
    int a = add_position(mesh, npositions, table, ax, ay, az);
    int b = add_position(mesh, npositions, table, bx, by, bz);
    int c = add_position(mesh, npositions, table, cx, cy, cz);
    int d = add_position(mesh, npositions, table, dx, dy, dz);

    if (a < 0 || b < 0 || c < 0 || d < 0) {
        return false;
    }
    set_triangle(&mesh->triangles[(*ntriangles)++], surface, a, b, c);
    set_triangle(&mesh->triangles[(*ntriangles)++], surface, a, c, d);
    return true;
}

bool world_build(world_mesh_t *mesh) {
    int table[1024];
    int npositions = 0;
    int ntriangles = 0;
    int slot;
    int x;
    int z;

    if (mesh == NULL) {
        return false;
    }
    for (slot = 0; slot < (int)(sizeof(table) / sizeof(table[0])); slot++) {
        table[slot] = -1;
    }
    for (z = -4000; z < 0; z += 250) {
        if (!set_quadrilateral(mesh, &npositions, table, &ntriangles, WORLD_MATERIAL_DIRT, -24, 0, z, 24, 0, z, 24, 0, z + 250, -24, 0, z + 250)) {
            return false;
        }
    }
    for (z = -4000; z < 0; z += 250) {
        for (x = -2000; x < 2000; x += 250) {
            if (!set_quadrilateral(mesh, &npositions, table, &ntriangles, WORLD_MATERIAL_GRASS, x, 0, z, x + 250, 0, z, x + 250, 0, z + 250, x, 0, z + 250)) {
                    return false;
            }
        }
    }
    if (!set_quadrilateral(mesh, &npositions, table, &ntriangles, WORLD_MATERIAL_WALL, 35, 0, -390, 65, 0, -390, 65, 20, -390, 35, 20, -390) ||
        !set_quadrilateral(mesh, &npositions, table, &ntriangles, WORLD_MATERIAL_WALL, 65, 0, -390, 65, 0, -425, 65, 20, -425, 65, 20, -390) ||
        !set_quadrilateral(mesh, &npositions, table, &ntriangles, WORLD_MATERIAL_ROOF, 32, 20, -387, 50, 29, -387, 50, 29, -428, 32, 20, -428) ||
        !set_quadrilateral(mesh, &npositions, table, &ntriangles, WORLD_MATERIAL_ROOF, 50, 29, -387, 68, 20, -387, 68, 20, -428, 50, 29, -428)) {
        return false;
    }
    if (npositions != (int)(sizeof(mesh->positions) / sizeof(mesh->positions[0])) || ntriangles != (int)(sizeof(mesh->triangles) / sizeof(mesh->triangles[0]))) {
        return false;
    }
    return true;
}
