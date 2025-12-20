#include "arguments_parsing.hpp"

static void parse_input(int argc, char* argv[], Arguments& args, int& i) {
    if (++i >= argc)
        throw std::invalid_argument("--input requires a filename");
    args.input_file = argv[i];
}

static void parse_output(int argc, char* argv[], Arguments& args, int& i) {
    if (++i >= argc)
        throw std::invalid_argument("--output requires a filename");
    args.output_file = argv[i];
}

static void parse_realization(int argc, char* argv[], Arguments& args, int& i) {
    if (++i >= argc)
        throw std::invalid_argument("--realization requires a value");
    args.realization = std::atoi(argv[i]);
    if (args.realization < 1 || args.realization > 3)
        throw std::invalid_argument("--realization must be 1, 2, or 3");
}

static void parse_threads(int argc, char* argv[], Arguments& args, int& i) {
    if (++i >= argc)
        throw std::invalid_argument("--threads requires a value");
    args.threads = std::atoi(argv[i]);
    if (args.threads <= 0)
        throw std::invalid_argument("--threads must be a positive integer");
}

static void parse_kind(int argc, char* argv[], Arguments& args, int& i) {
    if (++i >= argc)
        throw std::invalid_argument("--kind requires a value");
    std::string value = argv[i];
    if (value == "static") {
        args.kind = Arguments::ScheduleKind::Static;
    } else if (value == "dynamic") {
        args.kind = Arguments::ScheduleKind::Dynamic;
    } else {
        throw std::invalid_argument("--kind must be 'static' or 'dynamic'");
    }
}

static void parse_chunk_size(int argc, char* argv[], Arguments& args, int& i) {
    if (++i >= argc)
        throw std::invalid_argument("--chunk_size requires a value");

    args.chunk_size = std::atoi(argv[i]);

    if (args.chunk_size <= 0)
        throw std::invalid_argument("--chunk_size must be a positive integer");
}

void parse_arguments(int argc, char* argv[], Arguments& args) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--input") {
            parse_input(argc, argv, args, i);

        } else if (arg == "--output") {
            parse_output(argc, argv, args, i);

        } else if (arg == "--realization") {
            parse_realization(argc, argv, args, i);

        } else if (arg == "--threads") {
            parse_threads(argc, argv, args, i);

        } else if (arg == "--kind") {
            parse_kind(argc, argv, args, i);

        } else if (arg == "--chunk_size") {
            parse_chunk_size(argc, argv, args, i);

        } else { // ignoring }
    }
}
