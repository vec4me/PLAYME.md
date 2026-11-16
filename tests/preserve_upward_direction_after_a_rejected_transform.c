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
    -o build/preserve_upward_direction_after_a_rejected_transform_test || fail "could not build upward-direction test"
build/preserve_upward_direction_after_a_rejected_transform_test || fail "could not preserve upward direction after a rejected transform"
exit 0
#endif

#include <stdio.h>
#include "../playme/numeric.h"


int main(void) {
    numeric_matrix_t matrix;
    numeric_fixedvector_t direction = numeric_Direction(1, -1, 2);
    numeric_fixedvector_t original = direction;

    numeric_set_quaternion(&matrix, numeric_IdentityQuaternion());
    if (numeric_transform_upward_direction(&matrix, &direction)) {
        fprintf(stderr, "accepted downward direction\n");
        return 1;
    }
    if (numeric_RawValue(direction.x) != numeric_RawValue(original.x) || numeric_RawValue(direction.y) != numeric_RawValue(original.y) || numeric_RawValue(direction.z) != numeric_RawValue(original.z) ||
        numeric_RawValue(direction.w) != numeric_RawValue(original.w)) {
        fprintf(stderr, "rejected transform changed direction\n");
        return 1;
    }
    return 0;
}
