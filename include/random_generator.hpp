#pragma once

#include <random>
#include <cstdint>

class RandomGenerator {
public:
    RandomGenerator() : RandomGenerator(0) {}

    explicit RandomGenerator(int thread_id) {
        constexpr uint32_t base_seed = 0xdeadbeef;

        std::seed_seq seq{
            base_seed,
            static_cast<uint32_t>(thread_id),
            static_cast<uint32_t>(thread_id * 2654435761u)
        };

        rng.seed(seq);
    }

    inline float next() {
        return dist(rng);
    }

private:
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist{0.0f, 1.0f};
};
