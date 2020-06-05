#ifndef RNG_H
#define RNG_H

#include <cstdint>

// Source: https://en.wikipedia.org/wiki/Xorshift
struct rng_state {
    std::uint64_t state;
};

typedef struct rng_state rng_state_t;


std::uint64_t genRandom(rng_state_t& rng);

#endif