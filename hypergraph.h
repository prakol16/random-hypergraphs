#ifndef HYPERGRAPH_H
#define HYPERGRAPH_H

#include <cstdint>
#include <functional>
#include <vector>
#include <random>
#include <memory>
#include "hashgens.h"

// typedef std::function<Element(std::uint64_t maxHashValue, std::uint64_t value)> HashGen;

struct GraphVertex {
    std::uint64_t xormask;
    std::size_t count;
};

class HyperGraph {
    public:
        HyperGraph(std::size_t n, std::size_t m, std::mt19937_64& gen, std::shared_ptr<HashGen> hashGen);
        bool isPeelable();

    private:
        std::size_t m_n;
        std::size_t maxHashValue;
        std::vector<GraphVertex> vertices;
        std::shared_ptr<HashGen> m_hashGen;
};


#endif