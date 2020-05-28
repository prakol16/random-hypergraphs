#ifndef HASHGENS_H
#define HASHGENS_H

#include <cstdint>

struct Element {
    std::uint64_t h0;
    std::uint64_t h1;
    std::uint64_t h2;
};

class HashGen {
    public:
        virtual Element get(std::uint64_t value) const = 0;
};

class HashGenRandom : public HashGen {
    public:
        HashGenRandom(std::uint64_t m) : maxHashValue(m) {

        }

        Element get(std::uint64_t value) const;
    private:
        std::uint64_t maxHashValue;
};

class HashGenSegmented : public HashGen {
    public:
        HashGenSegmented(std::uint64_t m, std::size_t numSegments);
        Element get(std::uint64_t value) const;
        
    private:
        std::uint64_t maxHashValue;
        std::size_t m_numSegments;
        std::size_t segmentLength;  
};

class HashGenLimBandwidth : public HashGen {
    public:
        HashGenLimBandwidth(std::uint64_t m, std::size_t d) : maxHashValue(m), m_d(d) {}
        Element get(std::uint64_t value) const;
    
    private:
        std::uint64_t maxHashValue;
        std::size_t m_d;
};

#endif