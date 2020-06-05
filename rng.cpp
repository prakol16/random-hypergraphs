#include "rng.h"


std::uint64_t genRandom(rng_state_t& rng) {
    std::uint64_t x = rng.state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    rng.state = x;
    return x * 0x2545F4914F6CDD1D;
}
