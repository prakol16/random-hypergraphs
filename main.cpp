#include <iostream>
#include <random>
#include <cmath>
#include <bitset>
#include <cassert>
#include "hypergraph.h"
#include "hashgens.h"

static std::mt19937_64 gen(314159);


static std::size_t runTrials(std::size_t numTrials, std::size_t n, std::size_t m, std::shared_ptr<HashGen> hashGen) {
    std::size_t successes = 0;

    for (std::size_t i = 0; i < numTrials; ++i) {
        HyperGraph graph(n, m, gen, hashGen);
        if (graph.isPeelable()) successes++;
    }

    return successes;
}

// P vs m/n graph for many different n, basic hypergraph
static void generateBasicPvsMN() {
    std::uint64_t n = 100;
    for (std::size_t i = 2; i <= 5; ++i, n *= 10) {
        for (std::size_t mn_i = 0; mn_i < 10; ++mn_i) {
            double mn = 1.20 + mn_i * 0.01;
            std::size_t m = (std::size_t) (mn * n);
            auto hashGen = std::make_shared<HashGenRandom>(m);
            std::size_t successes = runTrials(100, n, m, hashGen);
            std::cout << "n=" << n << ", m=" << m << ", p=" << ((double)successes / 100.0) << std::endl;
        }
    }
} 

typedef std::function<std::shared_ptr<HashGen>(std::size_t)> HashGenFn;
static void findMinM(std::size_t n, double stepSize, const HashGenFn& getHashGen, std::size_t forceMultiple, double mn) {
    std::bitset<32> successTrack;
    int numSuccesses = 0;
    for (std::size_t i = 0; i < 1000; ++i) {
        std::size_t m = (std::size_t) (n * mn);
        m = (m / forceMultiple) * forceMultiple;
        HyperGraph graph(n, m, gen, getHashGen(m));
        if (successTrack[successTrack.size() - 1]) numSuccesses--;
        successTrack <<= 1;

        bool isPeelable = graph.isPeelable();
        if (isPeelable) {
            successTrack |= 1;
            numSuccesses++;
        }

        int half = successTrack.size() / 2;
        int diffFrom50 = numSuccesses >= half ? numSuccesses - half : half - numSuccesses;
        double step = stepSize * ((double)diffFrom50 / half);
        mn += step * (isPeelable ? -1 : 1);
    }
    std::cout << "n=" << n << " m/n=" << mn << std::endl;
}

static void generateBasicConvergenceGraph() {
    std::uint64_t n = 100;
    const HashGenFn fn = [](auto m) { return std::make_shared<HashGenRandom>(m); };
    for (std::size_t i = 2; i <= 6; ++i, n *= 10) {
        findMinM(n, 0.001, fn, 1, 1.22);
        findMinM(n * 3, 0.001, fn, 1, 1.22);
    }
}

static void generateSegmentConvergenceGraph() {
    const std::vector<std::size_t> ells = {12, 25, 50, 100, 150, 200};
    for (std::size_t ell : ells) {
        std::cout << "l=" << ell << std::endl;
        std::uint64_t n = 100;
        const HashGenFn fn = [ell](auto m) { return std::make_shared<HashGenSegmented>(m, ell); };
        for (std::size_t i = 2; i <= 6; ++i, n *= 10) {
            if (n > ell * 3)
                findMinM(n, 0.001, fn, ell, 1.12);
            if (n > ell)
                findMinM(n * 3, 0.001, fn, ell, 1.12);
        }
    }
}

static void generateBandLimConvergenceGraph() {
    const std::vector<std::size_t> bandQuotients = {6, 12, 25, 50, 100, 150};
    for (std::size_t band : bandQuotients) {
        std::cout << "band=" << band << std::endl;
        std::uint64_t n = 100;
        const HashGenFn fn = [band](auto m) { return std::make_shared<HashGenLimBandwidth>(m, m > 3*band ? m / band : 3); };
        for (std::size_t i = 2; i <= 6; ++i, n *= 10) {
            findMinM(n, 0.001, fn, 1, 1.12);
            findMinM(n * 3, 0.001, fn, 1, 1.12);
        }
    }
}

static void getEllGraphValuesFromN(std::size_t n, std::vector<std::size_t>& ells) {
    if (n == 100000) {
        for (std::size_t i = 20; i < 80; i += 20) ells.push_back(i);
        for (std::size_t i = 80; i < 120; i += 5) ells.push_back(i);
        for (std::size_t i = 120; i < 200; i += 20) ells.push_back(i);
    } else if (n == 30000) {
        ells.push_back(20);
        for (std::size_t i = 30; i < 70; i += 5) ells.push_back(i);
        for (std::size_t i = 70; i < 120; i += 10) ells.push_back(i);
        for (std::size_t i = 120; i < 200; i += 20) ells.push_back(i);
    } else if (n == 10000) {
        ells.push_back(10);
        for (std::size_t i = 21; i < 60; i += 3) ells.push_back(i);
        for (std::size_t i = 60; i < 200; i += 20) ells.push_back(i);
    } else {
        std::cout << "Don't know what l labels to use for n=" << n << std::endl;
    }
}

static void getBandGraphValuesFromN(std::size_t n, std::vector<std::size_t>& bands) {
    if (n == 100000) {
        bands.push_back(1);
        for (std::size_t i = 5; i < 25; i += 10) bands.push_back(i);
        for (std::size_t i = 25; i < 50; i += 5) bands.push_back(i);
        for (std::size_t i = 50; i < 100; i += 10) bands.push_back(i);
    } else {
        std::cout << "Don't know what band labels to use for n=" << n << std::endl;
    }
} 

static void generateOptimalEllGraphForN(std::size_t n) {
    std::vector<std::size_t> ells;
    getEllGraphValuesFromN(n, ells);

    for (std::size_t ell : ells) {
        const HashGenFn fn = [ell](auto m) { return std::make_shared<HashGenSegmented>(m, ell); };
        std::cout << "l=" << ell << " ";
        findMinM(n, 0.001, fn, ell, 1.12);
    }
}

static void generateOptimalBandGraphForN(std::size_t n) {
    std::vector<std::size_t> bands;
    getBandGraphValuesFromN(n, bands);

    for (std::size_t band : bands) {
        const HashGenFn fn = [band](auto m) { return std::make_shared<HashGenLimBandwidth>(m, m / band); };
        std::cout << "band=" << band << " ";
        findMinM(n, 0.001, fn, 1, 1.12);
    }
}

int main() {
    generateBasicPvsMN();
    generateBasicConvergenceGraph();
    generateSegmentConvergenceGraph();
    generateBandLimConvergenceGraph();
    generateOptimalEllGraphForN(10000);
    generateOptimalBandGraphForN(100000);
}
