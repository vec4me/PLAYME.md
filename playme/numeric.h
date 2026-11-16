#ifndef PLAYME_NUMERIC_H
#define PLAYME_NUMERIC_H

#if defined(__STDC_VERSION__) && __STDC_VERSION__ < 199901L
    #define inline __inline__
    #define bool int
    #define true 1
    #define false 0
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ < 202311L
    #include <stdbool.h>
#elif defined(__STDC_VERSION__)
#elif defined(__cplusplus)
#else
    #define inline __inline__
    #define bool int
    #define true 1
    #define false 0
#endif

#include <stdint.h>

typedef struct {
    int value;
} numeric_fixed_t;

typedef struct {
    int x;
    int y;
    int z;
    int w;
} numeric_vector_t;

typedef struct {
    numeric_fixed_t x;
    numeric_fixed_t y;
    numeric_fixed_t z;
    numeric_fixed_t w;
} numeric_fixedvector_t;

typedef struct {
    numeric_fixed_t x;
    numeric_fixed_t y;
    numeric_fixed_t z;
    numeric_fixed_t w;
} numeric_quaternion_t;

typedef struct {
    numeric_fixed_t xx;
    numeric_fixed_t yx;
    numeric_fixed_t zx;
    numeric_fixed_t wx;
    numeric_fixed_t xy;
    numeric_fixed_t yy;
    numeric_fixed_t zy;
    numeric_fixed_t wy;
    numeric_fixed_t xz;
    numeric_fixed_t yz;
    numeric_fixed_t zz;
    numeric_fixed_t wz;
    numeric_fixed_t xw;
    numeric_fixed_t yw;
    numeric_fixed_t zw;
    numeric_fixed_t ww;
} numeric_matrix_t;

typedef struct {
    numeric_fixed_t start;
    numeric_fixed_t end;
} numeric_scalar_t;

typedef struct {
    numeric_quaternion_t start;
    numeric_quaternion_t end;
} numeric_rotation_t;

#if defined(__cplusplus)
extern "C" {
#endif

int numeric_SquareRoot(unsigned radicand);
numeric_fixed_t numeric_FixedFromRaw(int raw);
extern inline int numeric_RawValue(numeric_fixed_t x) {
    return x.value;
}
numeric_fixed_t numeric_FixedFromInteger(int integer);
int numeric_IntegerValue(numeric_fixed_t x);
numeric_fixedvector_t numeric_Point(int x, int y, int z);
numeric_fixedvector_t numeric_Direction(int x, int y, int z);
numeric_fixed_t numeric_Sum(numeric_fixed_t a, numeric_fixed_t b);
numeric_fixed_t numeric_Difference(numeric_fixed_t a, numeric_fixed_t b);
numeric_fixed_t numeric_Negative(numeric_fixed_t x);
int numeric_RawSine(int angle);
numeric_fixed_t numeric_Sine(int angle);
int numeric_RawCosine(int angle);
numeric_fixed_t numeric_Cosine(int angle);
int numeric_Turn(int numerator, int denominator);
numeric_fixed_t numeric_Ratio(int numerator, int denominator);
extern inline int numeric_RawScale(int number, int amount) {
    unsigned product = (unsigned)number * (unsigned)amount;

    if ((number < 0) == (amount < 0)) {
        return (int)(product >> 10);
    }
    return -(int)((0U - product) >> 10);
}
numeric_fixed_t numeric_Product(numeric_fixed_t a, numeric_fixed_t b);
numeric_fixed_t numeric_IntegerProduct(numeric_fixed_t multiplicand, int multiplier);
int numeric_Quotient(numeric_fixed_t numerator, numeric_fixed_t denominator);
int numeric_ScaledValue(int number, numeric_fixed_t amount);
int numeric_Interpolation(int a, int b, numeric_fixed_t amount);
numeric_fixed_t numeric_FixedInterpolation(numeric_fixed_t a, numeric_fixed_t b, numeric_fixed_t amount);
numeric_quaternion_t numeric_IdentityQuaternion(void);
numeric_quaternion_t numeric_QuaternionProduct(numeric_quaternion_t a, numeric_quaternion_t b);
numeric_quaternion_t numeric_YXZQuaternion(int y, int x, int z);
numeric_fixed_t numeric_DotProduct(const numeric_fixedvector_t *a, const numeric_fixedvector_t *b);
numeric_fixed_t numeric_Norm(const numeric_fixedvector_t *v);
void numeric_normalize_quaternion(numeric_quaternion_t *q);
void numeric_normalize(numeric_fixedvector_t *v);
void numeric_set_perspective(numeric_matrix_t *matrix, int width, int height, int nearplane, int farplane);
void numeric_set_euler_yxz(numeric_matrix_t *matrix, int y, int x, int z);
void numeric_set_quaternion(numeric_matrix_t *matrix, numeric_quaternion_t rotation);
void numeric_transform(const numeric_matrix_t *matrix, numeric_fixedvector_t *v);
bool numeric_transform_upward_direction(const numeric_matrix_t *matrix, numeric_fixedvector_t *dir);
numeric_scalar_t numeric_Scalar(numeric_fixed_t start, numeric_fixed_t end);
numeric_rotation_t numeric_Rotation(numeric_quaternion_t start, numeric_quaternion_t end);
numeric_fixed_t numeric_Value(const numeric_scalar_t *curve, numeric_fixed_t amount);
numeric_quaternion_t numeric_RotationValue(const numeric_rotation_t *curve, numeric_fixed_t amount);
uint32_t numeric_SpatialCode(const numeric_fixedvector_t *pos, int seed);

#if defined(__cplusplus)
}
#endif

#endif
