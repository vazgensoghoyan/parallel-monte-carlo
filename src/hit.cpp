#include "hit.h"
#include <cmath>


bool hit_test(float x, float y, float z) {
    static const float a = 2.0f;

    float r2 = x * x + y * y;
    return r2 * r2 <= z * (x * x - y * y + a * z);
}

const float* get_axis_range() {
    static const float axis_range[6] = {-2.0f, 2.0f, -2.0f, 2.0f, 0.0f, 4.0f}; // x_min, x_max, y_min, y_max, z_min, z_max
    
    return axis_range;
}
