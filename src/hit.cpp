#include "hit.h"
#include <cmath>

static const float axis_range[6] = {
    0.0f, 2.0f,   // x_min, x_max
   -2.0f, 2.0f,   // y_min, y_max
   -2.0f, 2.0f    // z_min, z_max
};

bool hit_test(float x, float y, float z) {
    constexpr float a = 2.0f;

    if (x < 0.0f || x > a)
        return false;

    float x2 = x * x;
    float x3 = x2 * x;
    float x4 = x2 * x2;

    float value = (x4 - a * x3) + a * a * (y * y + z * z);

    return value <= 0.0f;
}

const float* get_axis_range() {
    return axis_range;
}
