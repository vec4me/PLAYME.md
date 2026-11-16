#if 0
fail() {
    printf '%s\n' "$*" >&2
    exit 1
}
cd "$(dirname "$0")/../" || fail "could not enter project directory"

mkdir -p build/ || fail "could not create build directory"
${CC:-cc} ${CFLAGS-} \
    "$0" \
    playme/numeric.c \
    -o build/preserve_unit_rotation_during_interpolation_test || fail "could not build rotation interpolation test"
build/preserve_unit_rotation_during_interpolation_test || fail "could not preserve unit rotation during interpolation"
exit 0
#endif

#include <stdio.h>
#include "../playme/numeric.h"

int main(void) {
    numeric_quaternion_t start = numeric_IdentityQuaternion();
    numeric_quaternion_t end = numeric_YXZQuaternion(numeric_Turn(1, 4), numeric_Turn(1, 8), 0);
    numeric_rotation_t curve;
    int step;

    numeric_normalize_quaternion(&end);
    curve = numeric_Rotation(start, end);
    for (step = 0; step <= 32; step++) {
        numeric_quaternion_t rotation;
        numeric_fixed_t amount;
        long norm;

        amount = numeric_Ratio(step, 32);
        rotation = numeric_RotationValue(&curve, amount);
        norm = (long)numeric_RawValue(rotation.x) * numeric_RawValue(rotation.x) +
               (long)numeric_RawValue(rotation.y) * numeric_RawValue(rotation.y) +
               (long)numeric_RawValue(rotation.z) * numeric_RawValue(rotation.z) +
               (long)numeric_RawValue(rotation.w) * numeric_RawValue(rotation.w);
        if (norm < 1024L * 1024L - 4096L || 1024L * 1024L + 4096L < norm) {
            fprintf(stderr, "interpolation step %d changed rotation norm to %ld\n", step, norm);
            return 1;
        }
    }
    return 0;
}
