#include "rng.hpp"


int RNG::getRandomInt(int max)
{
    std::uniform_int_distribution<int> dist(0, max);
    return dist(rng);
}

int RNG::getRandomInt(int min, int max)
{
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}

void RNG::setSeed(unsigned int seed)
{
    rng.seed(seed);
}