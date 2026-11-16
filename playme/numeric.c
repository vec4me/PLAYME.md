#include "numeric.h"

static const int fractionscale = 1024;

static const short sinevalues[256] = {
    0,    25,   51,   76,   101,  126,  151,  176,  201,  226,  250,   274,   298,   322,   346,   369,   392,   415,   438,   460,   482,   504,   526,   547,   568,   589,   609,   629,  648,
    667,  686,  704,  722,  740,  757,  774,  790,  805,  821,  835,   850,   863,   877,   889,   901,   913,   924,   935,   945,   954,   963,   971,   979,   986,   992,   998,   1004, 1008,
    1012, 1016, 1019, 1021, 1022, 1023, 1024, 1023, 1022, 1021, 1019, 1016,  1012,  1008,  1004,  998,   992,   986,   979,   971,   963,   954,   945,   935,   924,   913,   901,   889,  877,
    863,  850,  835,  821,  805,  790,  774,  757,  740,  722,  704,   686,   667,   648,   629,   609,   589,   568,   547,   526,   504,   482,   460,   438,   415,   392,   369,   346,  322,
    298,  274,  250,  226,  201,  176,  151,  126,  101,  76,   51,    25,    0,     -25,   -51,   -76,   -101,  -126,  -151,  -176,  -201,  -226,  -250,  -274,  -298,  -322,  -346,  -369, -392,
    -415, -438, -460, -482, -504, -526, -547, -568, -589, -609, -629, -648,  -667,  -686,  -704,  -722,  -740,  -757,  -774,  -790,  -805,  -821,  -835,  -850,  -863,  -877,  -889,  -901, -913,
    -924, -935, -945, -954, -963, -971, -979, -986, -992, -998, -1004, -1008, -1012, -1016, -1019, -1021, -1022, -1023, -1024, -1023, -1022, -1021, -1019, -1016, -1012, -1008, -1004, -998, -992,
    -986, -979, -971, -963, -954, -945, -935, -924, -913, -901, -889, -877,  -863,  -850,  -835,  -821,  -805,  -790,  -774,  -757,  -740,  -722,  -704,  -686,  -667,  -648,  -629,  -609, -589,
    -568, -547, -526, -504, -482, -460, -438, -415, -392, -369, -346, -322,  -298,  -274,  -250,  -226,  -201,  -176,  -151,  -126,  -101,  -76,   -51,   -25};

int numeric_SquareRoot(unsigned radicand) {
    unsigned root = 0;
    unsigned bit = 1 << 30;
    unsigned radicandrem = radicand;

    while (radicandrem < bit) {
        bit >>= 2;
    }
    while (bit != 0) {
        if (root + bit <= radicandrem) {
            radicandrem -= root + bit;
            root += bit << 1;
        }
        root >>= 1;
        bit >>= 2;
    }
    return (int)root;
}

numeric_fixed_t numeric_FixedFromRaw(int raw) {
    numeric_fixed_t x;

    x.value = raw;
    return x;
}

int numeric_RawValue(numeric_fixed_t x) {
    return x.value;
}

numeric_fixed_t numeric_FixedFromInteger(int integer) {
    return numeric_FixedFromRaw(integer * fractionscale);
}

int numeric_IntegerValue(numeric_fixed_t x) {
    int raw = x.value;

    if (raw < 0) {
        raw += fractionscale - 1;
    }
    return raw >> 10;
}

numeric_fixedvector_t numeric_Point(int x, int y, int z) {
    numeric_fixedvector_t point;

    point.x = numeric_FixedFromInteger(x);
    point.y = numeric_FixedFromInteger(y);
    point.z = numeric_FixedFromInteger(z);
    point.w = numeric_FixedFromInteger(1);
    return point;
}

numeric_fixedvector_t numeric_Direction(int x, int y, int z) {
    numeric_fixedvector_t dir;

    dir.x = numeric_FixedFromInteger(x);
    dir.y = numeric_FixedFromInteger(y);
    dir.z = numeric_FixedFromInteger(z);
    dir.w = numeric_FixedFromInteger(0);
    return dir;
}

numeric_fixed_t numeric_Sum(numeric_fixed_t a, numeric_fixed_t b) {
    return numeric_FixedFromRaw(a.value + b.value);
}

numeric_fixed_t numeric_Difference(numeric_fixed_t a, numeric_fixed_t b) {
    return numeric_FixedFromRaw(a.value - b.value);
}

numeric_fixed_t numeric_Negative(numeric_fixed_t x) {
    return numeric_FixedFromRaw(-x.value);
}

int numeric_RawSine(int angle) {
    return sinevalues[angle & 255];
}

numeric_fixed_t numeric_Sine(int angle) {
    return numeric_FixedFromRaw(numeric_RawSine(angle));
}

int numeric_RawCosine(int angle) {
    return sinevalues[(angle + 64) & 255];
}

numeric_fixed_t numeric_Cosine(int angle) {
    return numeric_FixedFromRaw(numeric_RawCosine(angle));
}

int numeric_Turn(int numerator, int denominator) {
    return numerator * 256 / denominator;
}

numeric_fixed_t numeric_Ratio(int numerator, int denominator) {
    return numeric_FixedFromRaw(numerator * fractionscale / denominator);
}

static inline int ScaledProduct(int a, int b) {
    unsigned product = (unsigned)a * (unsigned)b;

    if ((a < 0) == (b < 0)) {
        return (int)(product >> 10);
    }
    return -(int)((0U - product) >> 10);
}

int numeric_RawScale(int number, int amount) {
    unsigned product = (unsigned)number * (unsigned)amount;

    if ((number < 0) == (amount < 0)) {
        return (int)(product >> 10);
    }
    return -(int)((0U - product) >> 10);
}

static inline int RawProduct(numeric_fixed_t a, numeric_fixed_t b) {
    return ScaledProduct(a.value, b.value);
}

numeric_fixed_t numeric_Product(numeric_fixed_t a, numeric_fixed_t b) {
    return numeric_FixedFromRaw(RawProduct(a, b));
}

numeric_fixed_t numeric_IntegerProduct(numeric_fixed_t multiplicand, int multiplier) {
    return numeric_FixedFromRaw(multiplicand.value * multiplier);
}

int numeric_Quotient(numeric_fixed_t numerator, numeric_fixed_t denominator) {
    return numerator.value / denominator.value;
}

int numeric_ScaledValue(int number, numeric_fixed_t amount) {
    return numeric_RawScale(number, amount.value);
}

int numeric_Interpolation(int a, int b, numeric_fixed_t amount) {
    return a + numeric_ScaledValue(b - a, amount);
}

numeric_fixed_t numeric_FixedInterpolation(numeric_fixed_t a, numeric_fixed_t b, numeric_fixed_t amount) {
    return numeric_Sum(a, numeric_Product(numeric_Difference(b, a), amount));
}

numeric_quaternion_t numeric_IdentityQuaternion(void) {
    numeric_quaternion_t identity;

    identity.x = numeric_FixedFromInteger(0);
    identity.y = numeric_FixedFromInteger(0);
    identity.z = numeric_FixedFromInteger(0);
    identity.w = numeric_FixedFromInteger(1);
    return identity;
}

numeric_quaternion_t numeric_QuaternionProduct(numeric_quaternion_t a, numeric_quaternion_t b) {
    numeric_quaternion_t ab;

    ab.x = numeric_Sum(numeric_Sum(numeric_Product(a.w, b.x), numeric_Product(a.x, b.w)), numeric_Difference(numeric_Product(a.y, b.z), numeric_Product(a.z, b.y)));
    ab.y = numeric_Sum(numeric_Sum(numeric_Product(a.w, b.y), numeric_Product(a.y, b.w)), numeric_Difference(numeric_Product(a.z, b.x), numeric_Product(a.x, b.z)));
    ab.z = numeric_Sum(numeric_Sum(numeric_Product(a.w, b.z), numeric_Product(a.z, b.w)), numeric_Difference(numeric_Product(a.x, b.y), numeric_Product(a.y, b.x)));
    ab.w = numeric_Difference(numeric_Difference(numeric_Difference(numeric_Product(a.w, b.w), numeric_Product(a.x, b.x)), numeric_Product(a.y, b.y)), numeric_Product(a.z, b.z));
    return ab;
}

numeric_quaternion_t numeric_YXZQuaternion(int y, int x, int z) {
    numeric_quaternion_t qy = numeric_IdentityQuaternion();
    numeric_quaternion_t qx = numeric_IdentityQuaternion();
    numeric_quaternion_t qz = numeric_IdentityQuaternion();

    qy.y = numeric_Sine(y / 2);
    qy.w = numeric_Cosine(y / 2);
    qx.x = numeric_Sine(x / 2);
    qx.w = numeric_Cosine(x / 2);
    qz.z = numeric_Sine(z / 2);
    qz.w = numeric_Cosine(z / 2);
    return numeric_QuaternionProduct(numeric_QuaternionProduct(qy, qx), qz);
}

numeric_fixed_t numeric_DotProduct(const numeric_fixedvector_t *a, const numeric_fixedvector_t *b) {
    return numeric_Sum(numeric_Sum(numeric_Product(a->x, b->x), numeric_Product(a->y, b->y)), numeric_Product(a->z, b->z));
}

numeric_fixed_t numeric_Norm(const numeric_fixedvector_t *v) {
    unsigned vv = (unsigned)(v->x.value * v->x.value + v->y.value * v->y.value + v->z.value * v->z.value);

    return numeric_FixedFromRaw(numeric_SquareRoot(vv));
}

void numeric_normalize_quaternion(numeric_quaternion_t *q) {
    unsigned qq = (unsigned)(q->x.value * q->x.value + q->y.value * q->y.value + q->z.value * q->z.value + q->w.value * q->w.value);
    int normq = numeric_SquareRoot(qq);

    if (normq != 0) {
        q->x = numeric_Ratio(q->x.value, normq);
        q->y = numeric_Ratio(q->y.value, normq);
        q->z = numeric_Ratio(q->z.value, normq);
        q->w = numeric_Ratio(q->w.value, normq);
    }
}

void numeric_normalize(numeric_fixedvector_t *v) {
    numeric_fixed_t normv = numeric_Norm(v);

    if (normv.value != 0) {
        v->x = numeric_Ratio(v->x.value, normv.value);
        v->y = numeric_Ratio(v->y.value, normv.value);
        v->z = numeric_Ratio(v->z.value, normv.value);
    }
}

static void complete_rotation(numeric_matrix_t *matrix) {
    matrix->wx = numeric_FixedFromInteger(0);
    matrix->wy = numeric_FixedFromInteger(0);
    matrix->wz = numeric_FixedFromInteger(0);
    matrix->xw = numeric_FixedFromInteger(0);
    matrix->yw = numeric_FixedFromInteger(0);
    matrix->zw = numeric_FixedFromInteger(0);
    matrix->ww = numeric_FixedFromInteger(1);
}

void numeric_set_perspective(numeric_matrix_t *matrix, int width, int height, int nearplane, int farplane) {
    numeric_fixed_t zero = numeric_FixedFromInteger(0);

    matrix->xx = numeric_Ratio(height, width);
    matrix->yx = zero;
    matrix->zx = zero;
    matrix->wx = zero;
    matrix->xy = zero;
    matrix->yy = numeric_FixedFromInteger(1);
    matrix->zy = zero;
    matrix->wy = zero;
    matrix->xz = zero;
    matrix->yz = zero;
    matrix->zz = numeric_Ratio(-farplane, farplane - nearplane);
    matrix->wz = numeric_FixedFromInteger(-1);
    matrix->xw = zero;
    matrix->yw = zero;
    matrix->zw = numeric_Ratio(-farplane * nearplane, farplane - nearplane);
    matrix->ww = zero;
}

void numeric_set_euler_yxz(numeric_matrix_t *matrix, int y, int x, int z) {
    numeric_fixed_t cosiney = numeric_Cosine(y);
    numeric_fixed_t cosinex = numeric_Cosine(x);
    numeric_fixed_t cosinez = numeric_Cosine(z);
    numeric_fixed_t siney = numeric_Sine(y);
    numeric_fixed_t sinex = numeric_Sine(x);
    numeric_fixed_t sinez = numeric_Sine(z);

    matrix->xx = numeric_Sum(numeric_Product(cosiney, cosinez), numeric_Product(numeric_Product(siney, sinex), sinez));
    matrix->xy = numeric_Sum(numeric_Negative(numeric_Product(cosiney, sinez)), numeric_Product(numeric_Product(siney, sinex), cosinez));
    matrix->xz = numeric_Product(siney, cosinex);
    matrix->yx = numeric_Product(cosinex, sinez);
    matrix->yy = numeric_Product(cosinex, cosinez);
    matrix->yz = numeric_Negative(sinex);
    matrix->zx = numeric_Sum(numeric_Negative(numeric_Product(siney, cosinez)), numeric_Product(numeric_Product(cosiney, sinex), sinez));
    matrix->zy = numeric_Sum(numeric_Product(siney, sinez), numeric_Product(numeric_Product(cosiney, sinex), cosinez));
    matrix->zz = numeric_Product(cosiney, cosinex);
    complete_rotation(matrix);
}

void numeric_set_quaternion(numeric_matrix_t *matrix, numeric_quaternion_t rotation) {
    numeric_fixed_t two = numeric_FixedFromInteger(2);
    numeric_fixed_t xx = numeric_Product(rotation.x, rotation.x);
    numeric_fixed_t yy = numeric_Product(rotation.y, rotation.y);
    numeric_fixed_t zz = numeric_Product(rotation.z, rotation.z);
    numeric_fixed_t xy = numeric_Product(rotation.x, rotation.y);
    numeric_fixed_t xz = numeric_Product(rotation.x, rotation.z);
    numeric_fixed_t yz = numeric_Product(rotation.y, rotation.z);
    numeric_fixed_t wx = numeric_Product(rotation.w, rotation.x);
    numeric_fixed_t wy = numeric_Product(rotation.w, rotation.y);
    numeric_fixed_t wz = numeric_Product(rotation.w, rotation.z);

    matrix->xx = numeric_Difference(numeric_FixedFromInteger(1), numeric_Product(two, numeric_Sum(yy, zz)));
    matrix->xy = numeric_Product(two, numeric_Difference(xy, wz));
    matrix->xz = numeric_Product(two, numeric_Sum(xz, wy));
    matrix->yx = numeric_Product(two, numeric_Sum(xy, wz));
    matrix->yy = numeric_Difference(numeric_FixedFromInteger(1), numeric_Product(two, numeric_Sum(xx, zz)));
    matrix->yz = numeric_Product(two, numeric_Difference(yz, wx));
    matrix->zx = numeric_Product(two, numeric_Difference(xz, wy));
    matrix->zy = numeric_Product(two, numeric_Sum(yz, wx));
    matrix->zz = numeric_Difference(numeric_FixedFromInteger(1), numeric_Product(two, numeric_Sum(xx, yy)));
    complete_rotation(matrix);
}

void numeric_transform(const numeric_matrix_t *matrix, numeric_fixedvector_t *v) {
    numeric_fixedvector_t mv;
    unsigned p0;
    unsigned p1;
    unsigned p2;
    unsigned p3;

    p0 = (unsigned)matrix->xx.value * (unsigned)v->x.value;
    p1 = (unsigned)matrix->xy.value * (unsigned)v->y.value;
    p2 = (unsigned)matrix->xz.value * (unsigned)v->z.value;
    p3 = (unsigned)matrix->xw.value * (unsigned)v->w.value;
    mv.x.value = ((matrix->xx.value < 0) == (v->x.value < 0) ? (int)(p0 >> 10) : -(int)((0U - p0) >> 10)) +
                 ((matrix->xy.value < 0) == (v->y.value < 0) ? (int)(p1 >> 10) : -(int)((0U - p1) >> 10)) +
                 ((matrix->xz.value < 0) == (v->z.value < 0) ? (int)(p2 >> 10) : -(int)((0U - p2) >> 10)) +
                 ((matrix->xw.value < 0) == (v->w.value < 0) ? (int)(p3 >> 10) : -(int)((0U - p3) >> 10));

    p0 = (unsigned)matrix->yx.value * (unsigned)v->x.value;
    p1 = (unsigned)matrix->yy.value * (unsigned)v->y.value;
    p2 = (unsigned)matrix->yz.value * (unsigned)v->z.value;
    p3 = (unsigned)matrix->yw.value * (unsigned)v->w.value;
    mv.y.value = ((matrix->yx.value < 0) == (v->x.value < 0) ? (int)(p0 >> 10) : -(int)((0U - p0) >> 10)) +
                 ((matrix->yy.value < 0) == (v->y.value < 0) ? (int)(p1 >> 10) : -(int)((0U - p1) >> 10)) +
                 ((matrix->yz.value < 0) == (v->z.value < 0) ? (int)(p2 >> 10) : -(int)((0U - p2) >> 10)) +
                 ((matrix->yw.value < 0) == (v->w.value < 0) ? (int)(p3 >> 10) : -(int)((0U - p3) >> 10));

    p0 = (unsigned)matrix->zx.value * (unsigned)v->x.value;
    p1 = (unsigned)matrix->zy.value * (unsigned)v->y.value;
    p2 = (unsigned)matrix->zz.value * (unsigned)v->z.value;
    p3 = (unsigned)matrix->zw.value * (unsigned)v->w.value;
    mv.z.value = ((matrix->zx.value < 0) == (v->x.value < 0) ? (int)(p0 >> 10) : -(int)((0U - p0) >> 10)) +
                 ((matrix->zy.value < 0) == (v->y.value < 0) ? (int)(p1 >> 10) : -(int)((0U - p1) >> 10)) +
                 ((matrix->zz.value < 0) == (v->z.value < 0) ? (int)(p2 >> 10) : -(int)((0U - p2) >> 10)) +
                 ((matrix->zw.value < 0) == (v->w.value < 0) ? (int)(p3 >> 10) : -(int)((0U - p3) >> 10));

    p0 = (unsigned)matrix->wx.value * (unsigned)v->x.value;
    p1 = (unsigned)matrix->wy.value * (unsigned)v->y.value;
    p2 = (unsigned)matrix->wz.value * (unsigned)v->z.value;
    p3 = (unsigned)matrix->ww.value * (unsigned)v->w.value;
    mv.w.value = ((matrix->wx.value < 0) == (v->x.value < 0) ? (int)(p0 >> 10) : -(int)((0U - p0) >> 10)) +
                 ((matrix->wy.value < 0) == (v->y.value < 0) ? (int)(p1 >> 10) : -(int)((0U - p1) >> 10)) +
                 ((matrix->wz.value < 0) == (v->z.value < 0) ? (int)(p2 >> 10) : -(int)((0U - p2) >> 10)) +
                 ((matrix->ww.value < 0) == (v->w.value < 0) ? (int)(p3 >> 10) : -(int)((0U - p3) >> 10));
    *v = mv;
}

bool numeric_transform_upward_direction(const numeric_matrix_t *matrix, numeric_fixedvector_t *dir) {
    numeric_fixedvector_t mv;
    numeric_fixed_t x = dir->x;
    numeric_fixed_t y = dir->y;
    numeric_fixed_t z = dir->z;

    mv.y = numeric_FixedFromRaw(RawProduct(matrix->yx, x) + RawProduct(matrix->yy, y) + RawProduct(matrix->yz, z));
    if (mv.y.value <= 0) {
        return false;
    }
    mv.x = numeric_FixedFromRaw(RawProduct(matrix->xx, x) + RawProduct(matrix->xy, y) + RawProduct(matrix->xz, z));
    mv.z = numeric_FixedFromRaw(RawProduct(matrix->zx, x) + RawProduct(matrix->zy, y) + RawProduct(matrix->zz, z));
    dir->x = mv.x;
    dir->y = mv.y;
    dir->z = mv.z;
    return true;
}

numeric_scalar_t numeric_Scalar(numeric_fixed_t start, numeric_fixed_t end) {
    numeric_scalar_t scalar;

    scalar.start = start;
    scalar.end = end;
    return scalar;
}

numeric_rotation_t numeric_Rotation(numeric_quaternion_t start, numeric_quaternion_t end) {
    numeric_rotation_t rotation;

    rotation.start = start;
    rotation.end = end;
    return rotation;
}

numeric_fixed_t numeric_Value(const numeric_scalar_t *curve, numeric_fixed_t amount) {
    return numeric_FixedInterpolation(curve->start, curve->end, amount);
}

numeric_quaternion_t numeric_RotationValue(const numeric_rotation_t *curve, numeric_fixed_t amount) {
    numeric_quaternion_t end = curve->end;
    numeric_quaternion_t q;
    numeric_fixed_t dot;
    numeric_fixed_t startweight;
    numeric_fixed_t endweight;
    numeric_fixed_t sineangle;
    int angle = 0;
    int cand;
    int bestdiff = 512;

    dot = numeric_Sum(numeric_Sum(numeric_Product(curve->start.x, end.x), numeric_Product(curve->start.y, end.y)),
                      numeric_Sum(numeric_Product(curve->start.z, end.z), numeric_Product(curve->start.w, end.w)));
    if (numeric_RawValue(dot) < 0) {
        end.x = numeric_Negative(end.x);
        end.y = numeric_Negative(end.y);
        end.z = numeric_Negative(end.z);
        end.w = numeric_Negative(end.w);
        dot = numeric_Negative(dot);
    }
    for (cand = 0; cand <= 64; cand++) {
        int diff;

        diff = numeric_RawValue(numeric_Cosine(cand)) - numeric_RawValue(dot);
        if (diff < 0) {
            diff = -diff;
        }
        if (diff < bestdiff) {
            bestdiff = diff;
            angle = cand;
        }
    }
    sineangle = numeric_Sine(angle);
    if (angle == 0 || numeric_RawValue(sineangle) == 0) {
        q.x = numeric_FixedInterpolation(curve->start.x, end.x, amount);
        q.y = numeric_FixedInterpolation(curve->start.y, end.y, amount);
        q.z = numeric_FixedInterpolation(curve->start.z, end.z, amount);
        q.w = numeric_FixedInterpolation(curve->start.w, end.w, amount);
        numeric_normalize_quaternion(&q);
        return q;
    }
    startweight = numeric_Ratio(numeric_RawValue(numeric_Sine(angle - numeric_ScaledValue(angle, amount))), numeric_RawValue(sineangle));
    endweight = numeric_Ratio(numeric_RawValue(numeric_Sine(numeric_ScaledValue(angle, amount))), numeric_RawValue(sineangle));

    q.x = numeric_Sum(numeric_Product(curve->start.x, startweight), numeric_Product(end.x, endweight));
    q.y = numeric_Sum(numeric_Product(curve->start.y, startweight), numeric_Product(end.y, endweight));
    q.z = numeric_Sum(numeric_Product(curve->start.z, startweight), numeric_Product(end.z, endweight));
    q.w = numeric_Sum(numeric_Product(curve->start.w, startweight), numeric_Product(end.w, endweight));
    numeric_normalize_quaternion(&q);
    return q;
}

uint32_t numeric_SpatialCode(const numeric_fixedvector_t *pos, int seed) {
    int x = numeric_IntegerValue(pos->x);
    int z = numeric_IntegerValue(pos->z);
    uint32_t code = (uint32_t)x * 73856093u ^ (uint32_t)z * 19349663u ^ (uint32_t)seed * 83492791u;

    return (code ^ (code >> 13)) * 1274126177u;
}
