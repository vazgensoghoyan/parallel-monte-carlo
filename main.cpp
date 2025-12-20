#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <vector>
#include <omp.h>

#include "hit.h"
#include "arguments_parsing.hpp"

#include <string>
#include <stdexcept>
#include <cstdlib>

double monte_carlo_volume(size_t N) {
    const float* ranges = get_axis_range();
    float x_min = ranges[0], x_max = ranges[1];
    float y_min = ranges[2], y_max = ranges[3];
    float z_min = ranges[4], z_max = ranges[5];

    double hits = 0;

    for (size_t i = 0; i < N; ++i) {
        float x = x_min + static_cast<float>(rand()) / RAND_MAX * (x_max - x_min);
        float y = y_min + static_cast<float>(rand()) / RAND_MAX * (y_max - y_min);
        float z = z_min + static_cast<float>(rand()) / RAND_MAX * (z_max - z_min);

        if (hit_test(x, y, z)) {
            hits += 1.0;
        }
    }

    double volume_box = (x_max - x_min) * (y_max - y_min) * (z_max - z_min);
    return volume_box * hits / N;
}

void write_result(const std::string& filename, double volume) {
    std::ofstream fout(filename);
    if (!fout) {
        throw std::runtime_error("Failed to open output file: " + filename);
    }

    fout << volume << "\n";
}

double calculate_volume(size_t N, const Arguments& args) {
    double volume = 0.0;

    if (args.realization == 1) {
        volume = monte_carlo_volume(N);
    } else {
        throw std::logic_error("This realization is not implemented yet");
    }
}

int main(int argc, char* argv[]) {
    try {
        auto args = Arguments::parse_arguments(argc, argv);

        size_t N = args.read_N();

        volatile double start_time = omp_get_wtime();
        double volume = calculate_volume(N, args);
        volatile double end_time = omp_get_wtime();

        write_result(args.output_file, volume);

        double elapsed_ms = (end_time - start_time) * 1000.0;
        std::cout << "Time (0 thread(s)): " << elapsed_ms << " ms\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
