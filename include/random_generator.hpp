#pragma once

#include <cstdint>
#include <chrono>

class RandomGenerator {
public:
    explicit RandomGenerator(uint64_t seed = 0) {
        seed += std::chrono::steady_clock::now().time_since_epoch().count();
        state = 0;
        increment = (seed << 1u) | 1u; // гарантируем нечётный increment
        next();
        state += seed;
        next();
    }

    // возвращает 32-битное случайное число
    uint32_t next() {
        uint64_t oldstate = state;
        state = oldstate * 6364136223846793005ULL + increment;
        uint32_t xorshifted = static_cast<uint32_t>(((oldstate >> 18u) ^ oldstate) >> 27u);
        uint32_t rot = static_cast<uint32_t>(oldstate >> 59u);
        return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
    }

    // случайное число в [0, 1)
    float next_float() {
        return next() * (1.0f / 4294967296.0f); // 2^32
    }

private:
    uint64_t state;
    uint64_t increment;
};
