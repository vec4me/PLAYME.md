#if 0
fail() {
    printf '%s\n' "$*" >&2
    exit 1
}
cd "$(dirname "$0")/../" || fail "could not enter project directory"

mkdir -p build/ || fail "could not create build directory"
${CC:-cc} ${CFLAGS-} "$0" playme/renderer/world.c ${LDFLAGS-} ${LDLIBS-} -o build/create_the_static_map_test || fail "could not build static world test"
build/create_the_static_map_test || fail "could not create the static map"
exit 0
#endif

#include <stdio.h>
#include "../playme/renderer/world.h"

int main(void) {
    world_mesh_t mesh;
    world_mesh_t rebuilt;
    int npositions = (int)(sizeof(mesh.positions) / sizeof(mesh.positions[0]));
    int ntriangles = (int)(sizeof(mesh.triangles) / sizeof(mesh.triangles[0]));
    int tri;
    int pos;
    int vertex;

    if (world_build(NULL)) {
        return 1;
    }
    if (!world_build(&mesh)) {
        fprintf(stderr, "could not create map\n");
        return 1;
    }
    for (tri = 0; tri < ntriangles; tri++) {
        for (vertex = 0; vertex < 3; vertex++) {
            if (npositions <= mesh.triangles[tri].vertices[vertex]) {
                fprintf(stderr, "world triangle referenced an invalid position\n");
                return 1;
            }
        }
    }
    if (!world_build(&rebuilt)) {
        fprintf(stderr, "could not rebuild static world\n");
        return 1;
    }
    for (pos = 0; pos < npositions; pos++) {
        if (mesh.positions[pos].x != rebuilt.positions[pos].x || mesh.positions[pos].y != rebuilt.positions[pos].y || mesh.positions[pos].z != rebuilt.positions[pos].z) {
            fprintf(stderr, "rebuilding changed a static world position\n");
            return 1;
        }
    }
    for (tri = 0; tri < ntriangles; tri++) {
        if (mesh.triangles[tri].surface != rebuilt.triangles[tri].surface) {
            fprintf(stderr, "rebuilding changed a static world material\n");
            return 1;
        }
        for (vertex = 0; vertex < 3; vertex++) {
            if (mesh.triangles[tri].vertices[vertex] != rebuilt.triangles[tri].vertices[vertex]) {
                fprintf(stderr, "rebuilding changed a static world triangle\n");
                return 1;
            }
        }
    }
    return 0;
}
