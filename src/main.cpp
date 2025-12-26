#include <iostream>
#include <fstream>
#include <cstdlib>

#include "hit.h"
#include "arguments_parsing.hpp"
#include "monte_carlos.hpp"

void write_result(const std::string& filename, double volume) {
    std::ofstream fout(filename);
    if (!fout) {
        throw std::runtime_error("Failed to open output file: " + filename);
    }
    fout << volume << "\n";
}

int main(int argc, char* argv[]) {
    try {
        auto args = Arguments::parse_arguments(argc, argv);

        size_t N = args.read_N();

        double volume = 0.0f;

        volatile double start_time = omp_get_wtime();
        volume = calculate_volume(N, args);
        volatile double end_time = omp_get_wtime();
        
        double total_time = (end_time - start_time) * 1000.0;

        write_result(args.output_file, volume);

        int num_threads = 0;
        if (args.realization > 1)
            num_threads = args.threads > 0 ? args.threads : omp_get_max_threads();

        std::cout << "Time (" << num_threads << " thread(s)): " << total_time << " ms\n";

    } catch (const std::exception& e) {
        std::cerr << "[ERROR]: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
