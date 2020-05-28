#include "hashgens.h"
#include <cassert>


// Scramble a 64-bit number
static std::uint64_t murmur64(std::uint64_t h) {
	h ^= h >> 33;
	h *= 0xff51afd7ed558ccd;
	h ^= h >> 33;
	h *= 0xc4ceb9fe1a85ec53;
	h ^= h >> 33;
	return h;
}

// Requires v to be a random int in the range [0, 2^32)
// Computes a random integer in [0, m) based on v
// Assumes m < 2^32
static std::uint64_t reduce(std::uint64_t v, std::uint64_t m) {
    return (v * m) >> 32;
}

static const std::uint64_t MASK_32 = 0xFFFFFFFF;

// Sets remaining bits to h3
static Element get3DistinctValues(std::uint64_t value, std::uint64_t max, std::uint64_t& h3) {
    Element e;
    // std::size_t h0 = murmur64(value), h1 = murmur64(h0), h2 = murmur64(h1);
    std::size_t h0 = value & MASK_32, h1 = value >> 32, mur = murmur64(value), h2 = mur & MASK_32;
    h3 = mur >> 32;
    std::size_t h1diff = 1 + reduce(h1, max - 1);
    std::size_t h2diff = 2 + reduce(h2, max - 2);

    e.h0 = reduce(h0, max);
    if (h2diff <= h1diff) --h2diff;
    e.h1 = e.h0 + h1diff;
    if (e.h1 >= max) e.h1 -= max;
    e.h2 = e.h0 + h2diff;
    if (e.h2 >= max) e.h2 -= max;
    return e;
}

// Creates a ``dummy'' h3 to dispose of the remaining bits
static Element get3DistinctValues(std::uint64_t value, std::uint64_t max) {
    std::size_t h3;
    return get3DistinctValues(value, max, h3);
}


Element HashGenRandom::get(std::uint64_t value) const {
    return get3DistinctValues(value, maxHashValue);
}


HashGenSegmented::HashGenSegmented(std::uint64_t m, std::size_t numSegments) : maxHashValue(m), m_numSegments(numSegments) {
    assert(m % numSegments == 0);
    segmentLength = m / numSegments;
}

Element HashGenSegmented::get(std::uint64_t value) const {
    Element e;
    // std::size_t h0 = murmur64(value), h1 = murmur64(h0), h2 = murmur64(h1), h3 = murmur64(h2);
    std::size_t h0 = value & MASK_32, h1 = value >> 32, mur = murmur64(value), h2 = mur & MASK_32, h3 = mur >> 32;
    std::size_t segStart = h0 % (m_numSegments - 2);
    e.h0 = segStart * segmentLength + (h1 % segmentLength);
    e.h1 = (segStart + 1) * segmentLength + (h2 % segmentLength);
    e.h2 = (segStart + 2) * segmentLength + (h3 % segmentLength);
    return e;
}

Element HashGenLimBandwidth::get(std::uint64_t value) const {
    Element e;
    std::size_t h0;
    e = get3DistinctValues(value, m_d, h0);
    std::size_t start = reduce(h0, maxHashValue - m_d + 1);
    e.h0 += start;
    e.h1 += start;
    e.h2 += start;
    return e;
}