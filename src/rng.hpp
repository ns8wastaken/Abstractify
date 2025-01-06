#pragma once
#include <random>

class RNG
{
public:
    RNG(unsigned int seed = std::random_device{}())
        : rng(seed)
    {}

    // Generate a random integer between 0 and max (inclusive)
    int getRandomInt(int max);

    // Generate a random integer between min and max (inclusive)
    int getRandomInt(int min, int max);

    void setSeed(unsigned int seed);

private:
    std::mt19937 rng;
};
