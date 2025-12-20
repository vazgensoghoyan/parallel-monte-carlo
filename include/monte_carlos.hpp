#pragma once

#include <cstddef>
#include <stdexcept>
#include <vector>
#include <atomic>
#include <omp.h>

#include "hit.h"
#include "arguments_parsing.hpp"
#include "random_generator.hpp"

double monte_carlo_single(size_t N);

double monte_carlo_auto_parallel(size_t N, const Arguments& args);

double monte_carlo_manual_parallel(size_t N, const Arguments& args);

double calculate_volume(size_t N, const Arguments& args);
