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

double calculate_volume(size_t N, const Arguments& args) {
    double volume = 0.0;

    switch (args.realization) {
        case 1:
            volume = monte_carlo_single(N);
            break;
        case 2:
            volume = monte_carlo_auto_parallel(N, args);
            break;
        case 3:
            volume = monte_carlo_manual_parallel(N, args);
            break;
        default:
            throw std::invalid_argument("Invalid realization number");
    }

    return volume;
}

int main(int argc, char* argv[]) {
    try {
        auto args = Arguments::parse_arguments(argc, argv);

        size_t N = args.read_N();

        double start_time = omp_get_wtime();
        double volume = calculate_volume(N, args);
        double end_time = omp_get_wtime();

        write_result(args.output_file, volume);

        int num_threads = 0;
        if (args.realization > 1) 
            num_threads = args.threads > 0 ? args.threads : omp_get_max_threads();
        
            double elapsed_ms = (end_time - start_time) * 1000.0;
        std::cout << "Time (" << num_threads << " thread(s)): " << elapsed_ms << " ms\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
