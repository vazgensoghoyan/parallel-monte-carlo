#pragma once

#include <random>

class RandomGenerator {
public:
    RandomGenerator() : rng(std::random_device{}()), dist(0.0f, 1.0f) {}

    // random in [0,1)
    float next() {
        return dist(rng);
    }

private:
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist;
};
