#include "hypergraph.h"
#include "hashgens.h"

HyperGraph::HyperGraph(std::size_t n, std::size_t m, std::mt19937_64& gen, std::shared_ptr<HashGen> hashGen) 
                : m_n(n), maxHashValue(m), m_hashGen(hashGen){
    vertices.resize(m);
    for (std::size_t i = 0; i < n; ++i) {
        std::uint64_t value = gen();
        Element e = m_hashGen->get(value);
        vertices[e.h0].count++;
        vertices[e.h0].xormask ^= value;
        vertices[e.h1].count++;
        vertices[e.h1].xormask ^= value;
        vertices[e.h2].count++;
        vertices[e.h2].xormask ^= value;
    }
}

struct QElem {
    std::size_t index;
    std::uint64_t hash;
};

// Code adapted from https://github.com/FastFilter/xorfilter/blob/master/fusefilter.go
bool HyperGraph::isPeelable() {
    std::vector<QElem> queue;
    for (std::size_t i = 0; i < vertices.size(); ++i) {
        const GraphVertex& vtx = vertices[i];
        if (vtx.count == 1) {
            queue.push_back({i, vtx.xormask});
        }
    }

    while (queue.size() > 0) {
        QElem qelem = queue[queue.size() - 1];
        queue.pop_back();
        if (vertices[qelem.index].count == 0) continue;

        Element e = m_hashGen->get(qelem.hash);
        vertices[e.h0].xormask ^= qelem.hash;
        vertices[e.h0].count--;
        if (vertices[e.h0].count == 1) queue.push_back({e.h0, vertices[e.h0].xormask});

        vertices[e.h1].xormask ^= qelem.hash;
        vertices[e.h1].count--;
        if (vertices[e.h1].count == 1) queue.push_back({e.h1, vertices[e.h1].xormask});

        vertices[e.h2].xormask ^= qelem.hash;
        vertices[e.h2].count--;
        if (vertices[e.h2].count == 1) queue.push_back({e.h2, vertices[e.h2].xormask});

        --m_n;
        if (m_n == 0) return true;
    }
    return false;
}