#pragma once

#include <string>
#include <stdexcept>

enum class ScheduleKind { Auto, Static, Dynamic };

class Arguments {

public:
    static Arguments parse_arguments(int argc, char* argv[]);

public:
    std::string input_file = "input.txt";
    std::string output_file = "output.txt";
    int realization = 1;

    int threads = -1;              // -1 = использовать OpenMP default
    ScheduleKind kind = ScheduleKind::Auto;
    int chunk_size = -1;           // -1 = оптимальный

};

