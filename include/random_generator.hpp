#pragma once

#include <random>
#include <cstdint>

class RandomGenerator {

    public:
    explicit RandomGenerator(uint64_t seed = 0) : state(seed ? seed : 0xDEADBEEF) {}

    uint64_t next() {
        uint64_t x = state;

        x ^= x << 13;
        x ^= x >> 7;
        x ^= x << 17;

        state = x;
        return x;
    }

    float next_float() {
        return (next() >> 40) * (1.0f / 16777216.0f);
    }

private:
    uint64_t state;
};
