#include "monte_carlos.hpp"

double monte_carlo_single(size_t N) {
    const float* ranges = get_axis_range();
    float x_min = ranges[0], x_max = ranges[1];
    float y_min = ranges[2], y_max = ranges[3];
    float z_min = ranges[4], z_max = ranges[5];

    RandomGenerator rng;
    double hits = 0;

    for (size_t i = 0; i < N; ++i) {
        float x = x_min + rng.next() * (x_max - x_min);
        float y = y_min + rng.next() * (y_max - y_min);
        float z = z_min + rng.next() * (z_max - z_min);

        if (hit_test(x, y, z)) {
            hits += 1.0;
        }
    }

    double volume_box = (x_max - x_min) * (y_max - y_min) * (z_max - z_min);
    return volume_box * hits / N;
}

double monte_carlo_auto_parallel(size_t N) {
    throw std::logic_error("Realization 2 not implemented yet");
}

double monte_carlo_manual_parallel(size_t N) {
    throw std::logic_error("Realization 3 not implemented yet");
}
