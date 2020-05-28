#include <iostream>
#include <random>
#include <cmath>
#include "hypergraph.h"
#include "hashgens.h"

// static void initializeLoadingBar(std::size_t numLoads) {
//     std::cout << "[";
//     for (std::size_t i = 0; i < numLoads; ++i) std::cout << "-";
//     std::cout << "]";
//     for (std::size_t i = 0; i < numLoads + 1; ++i) std::cout << "\b";
//     std::cout.flush();
// }

static void runTrials(std::size_t numTrials, std::size_t n, std::size_t m, std::shared_ptr<HashGen> hashGen, std::mt19937_64& gen) {
    std::size_t successes = 0;

    for (std::size_t i = 0; i < numTrials; ++i) {
        HyperGraph graph(n, m, gen, hashGen);
        if (graph.isPeelable()) successes++;
    }

    double p = ((double)successes / (double)numTrials);
    double err = sqrt(p*(1-p) / ((double) numTrials));
    std::cout << "Success p=" << p << "; std. error=" << err << std::endl;
}


int main() {
    std::mt19937_64 gen(314159);
    std::size_t n = 100000, m;
    // for (m = 122000; m < 124000; m += 200) {
    //     auto hashGenRandom = std::make_shared<HashGenRandom>(m);
    //     std::cout << "Random (n=" << n << ", m=" << m << "): ";
    //     runTrials(100, n, m, hashGenRandom, gen);
    // }
    // for (m = 116000; m < 118000; m += 200) {
    //     auto hashGenSegmented = std::make_shared<HashGenSegmented>(m, 200);
    //     std::cout << "Segmented (n=" << n << ", m=" << m << ", l=200): ";
    //     runTrials(100, n, m, hashGenSegmented, gen);
    // }
    // for (m = 115000; m < 117000; m += 200) {
    //     auto hashGenSegmented = std::make_shared<HashGenSegmented>(m, 100);
    //     std::cout << "Segmented (n=" << n << ", m=" << m << ", l=100): ";
    //     runTrials(100, n, m, hashGenSegmented, gen);
    // }
    // for (m = 115000; m < 117000; m += 200) {
    //     auto hashGenSegmented = std::make_shared<HashGenSegmented>(m, 50);
    //     std::cout << "Segmented (n=" << n << ", m=" << m << ", l=50): ";
    //     runTrials(100, n, m, hashGenSegmented, gen);
    // }
    // for (m = 119000; m < 121000; m += 200) {
    //     auto hashGenSegmented = std::make_shared<HashGenSegmented>(m, 25);
    //     std::cout << "Segmented (n=" << n << ", m=" << m << ", l=25): ";
    //     runTrials(100, n, m, hashGenSegmented, gen);
    // }
    for (m = 117000; m < 119000; m += 200) {
        auto hashGenBandwidth = std::make_shared<HashGenLimBandwidth>(m, m / 1000);
        std::cout << "Lim. Bandwidth (n=" << n << ", m=" << m << ", d=" << (m / 100) << "): ";
        runTrials(100, n, m, hashGenBandwidth, gen);
    }
}
