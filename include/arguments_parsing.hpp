#pragma once

#include <string>
#include <stdexcept>


struct Arguments {
    std::string input_file = "input.txt";
    std::string output_file = "output.txt";
    int realization = 1;

    int threads = -1;              // -1 = использовать OpenMP default
    enum class ScheduleKind { Auto, Static, Dynamic } kind = ScheduleKind::Auto;
    int chunk_size = -1;           // -1 = оптимальный
};

void parse_arguments(int argc, char* argv[], Arguments& args);
