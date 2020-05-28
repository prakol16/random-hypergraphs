#include "hashgens.h"
#include <cassert>

static std::uint64_t murmur64(std::uint64_t h) {
	h ^= h >> 33;
	h *= 0xff51afd7ed558ccd;
	h ^= h >> 33;
	h *= 0xc4ceb9fe1a85ec53;
	h ^= h >> 33;
	return h;
}

static Element get3DistinctValues(std::uint64_t value, std::uint64_t max) {
    Element e;
    std::size_t h0 = murmur64(value), h1 = murmur64(h0), h2 = murmur64(h1);
    std::size_t h1diff = 1 + (h1 % (max - 1));
    std::size_t h2diff = 2 + (h2 % (max - 2));

    e.h0 = h0 % max;
    if (h2diff <= h1diff) --h2diff;
    e.h1 = e.h0 + h1diff;
    if (e.h1 >= max) e.h1 -= max;
    e.h2 = e.h0 + h2diff;
    if (e.h2 >= max) e.h2 -= max;
    return e;
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
    std::size_t h0 = murmur64(value), h1 = murmur64(h0), h2 = murmur64(h1), h3 = murmur64(h2);
    std::size_t segStart = h0 % (m_numSegments - 2);
    e.h0 = segStart * segmentLength + (h1 % segmentLength);
    e.h1 = (segStart + 1) * segmentLength + (h2 % segmentLength);
    e.h2 = (segStart + 2) * segmentLength + (h3 % segmentLength);
    return e;
}

Element HashGenLimBandwidth::get(std::uint64_t value) const {
    Element e;
    std::size_t h0 = murmur64(value);
    e = get3DistinctValues(h0, m_d);
    std::size_t start = h0 % (maxHashValue - m_d + 1);
    e.h0 += start;
    e.h1 += start;
    e.h2 += start;
    return e;
}