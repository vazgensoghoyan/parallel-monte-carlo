#pragma once

#include <string>
#include <stdexcept>
#include <fstream>
#include <cstdlib>

enum class ScheduleKind { Auto, Static, Dynamic };

class Arguments {

public:
    static Arguments parse_arguments(int argc, char* argv[]);

    size_t read_N() const;

public:
    std::string input_file;
    std::string output_file;
    int realization;

    int threads = -1;              // -1 = использовать OpenMP default
    ScheduleKind kind = ScheduleKind::Auto;
    int chunk_size = -1;           // -1 = оптимальный

};
