#include <iostream>
#include <iomanip>
#include <random>
#include <cmath>
#include <bitset>
#include <cassert>
#include <functional>
#include <algorithm>
#include "hypergraph.h"
#include "hashgens.h"
#include "rng.h"

// static std::mt19937_64 gen(314159);

rng_state_t gen = {0xd76b564f3d5db25};

static const double STEP_SIZE = 0.01;
static const double BASIC_LOW_BOUND = 1.22;
static const double ORIENTABLE_LOW_BOUND = 1.12;
static const double STEP_DECAY = 0.6;
static constexpr std::size_t KEEP_LAST_N = 32;
static std::array<double, KEEP_LAST_N/2 +1> powCache;

static void fillPowCache() {
    double d = 1.0;
    for (std::size_t i = 0; i <= KEEP_LAST_N / 2; ++i, d *= STEP_DECAY) powCache[i] = d;
}

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
            std::cout << "n=" << n << " m=" << m << " p=" << ((double)successes / 100.0) << std::endl;
        }
    }
} 

typedef std::function<std::shared_ptr<HashGen>(std::size_t, std::size_t)> HashGenFn;

std::shared_ptr<HashGen> makeOrdinaryGraph(std::size_t m, std::size_t unused) {
    return std::make_shared<HashGenRandom>(m);
}

std::shared_ptr<HashGen> makeSegmentedGraph(std::size_t m, std::size_t ell) {
    return std::make_shared<HashGenSegmented>(m, ell);
}

std::shared_ptr<HashGen> makeBandedGraph(std::size_t m, std::size_t band) {
    return std::make_shared<HashGenLimBandwidth>(m, m > 3*band ? m / band : 3);
}

static double findMinM(std::size_t n, double stepSize, const HashGenFn& getHashGen, std::size_t forceMultiple, double mn, std::size_t param) {
    std::bitset<KEEP_LAST_N> successTrack;
    int numSuccesses = 0;
    for (std::size_t i = 0; i < 1000; ++i) {
        std::size_t m = (std::size_t) (n * mn);
        m = (m / forceMultiple) * forceMultiple;
        HyperGraph graph(n, m, gen, getHashGen(m, param));
        if (successTrack[KEEP_LAST_N - 1]) numSuccesses--;
        successTrack <<= 1;

        bool isPeelable = graph.isPeelable();
        if (isPeelable) {
            successTrack |= 1;
            numSuccesses++;
        }

        const int half = KEEP_LAST_N / 2;
        int diffFrom50 = numSuccesses >= half ? (KEEP_LAST_N - numSuccesses) : numSuccesses;
        double step = stepSize * powCache[diffFrom50];
        mn += step * (isPeelable ? -1 : 1);
    }
    std::cout << "n=" << n << " m/n=" << mn << std::endl;
    return mn;
}

static void generateBasicConvergenceGraph() {
    std::uint64_t n = 100;
    for (std::size_t i = 2; i <= 6; ++i, n *= 10) {
        findMinM(n, STEP_SIZE, makeOrdinaryGraph, 1, BASIC_LOW_BOUND, 0);
        findMinM(n * 3, STEP_SIZE, makeOrdinaryGraph, 1, BASIC_LOW_BOUND, 0);
    }
}

static void generateSegmentConvergenceGraph() {
    const std::vector<std::size_t> ells = {12, 25, 50, 100, 150, 200};
    for (std::size_t ell : ells) {
        std::cout << "l=" << ell << std::endl;
        std::uint64_t n = 100;
        for (std::size_t i = 2; i <= 6; ++i, n *= 10) {
            if (n > ell * 3)
                findMinM(n, STEP_SIZE, makeSegmentedGraph, ell, ORIENTABLE_LOW_BOUND, ell);
            if (n > ell)
                findMinM(n * 3, STEP_SIZE, makeSegmentedGraph, ell, ORIENTABLE_LOW_BOUND, ell);
        }
    }
}

static void generateBandLimConvergenceGraph() {
    const std::vector<std::size_t> bandQuotients = {6, 12, 25, 50, 100, 150};
    for (std::size_t band : bandQuotients) {
        std::cout << "band=" << band << std::endl;
        std::uint64_t n = 100;
        for (std::size_t i = 2; i <= 6; ++i, n *= 10) {
            findMinM(n, STEP_SIZE, makeBandedGraph, 1, ORIENTABLE_LOW_BOUND, band);
            findMinM(n * 3, STEP_SIZE, makeBandedGraph, 1, ORIENTABLE_LOW_BOUND, band);
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
    bands.push_back(1);
    if (n == 100000) {
        for (std::size_t i = 5; i < 25; i += 10) bands.push_back(i);
        for (std::size_t i = 25; i < 50; i += 5) bands.push_back(i);
        for (std::size_t i = 50; i < 100; i += 10) bands.push_back(i);
    } else if (n == 30000) {
        for (std::size_t i = 5; i <= 10; i += 5) bands.push_back(i);
        for (std::size_t i = 15; i < 45; i += 5) bands.push_back(i);
        for (std::size_t i = 50; i < 100; i += 10) bands.push_back(i);
    } else if (n == 300000) {
        for (std::size_t i = 10; i < 30; i += 10) bands.push_back(i);
        for (std::size_t i = 35; i < 75; i += 5) bands.push_back(i);
        for (std::size_t i = 80; i < 100; i += 10) bands.push_back(i);
    } else {
        std::cout << "Don't know what band labels to use for n=" << n << std::endl;
    }
} 

static void generateOptimalEllGraphForN(std::size_t n) {
    std::vector<std::size_t> ells;
    getEllGraphValuesFromN(n, ells);
    std::cout << "n=" << n << std::endl;
    for (std::size_t ell : ells) {
        std::cout << "l=" << ell << " ";
        findMinM(n, STEP_SIZE, makeSegmentedGraph, ell, ORIENTABLE_LOW_BOUND, ell);
    }
}

static void generateOptimalBandGraphForN(std::size_t n) {
    std::vector<std::size_t> bands;
    getBandGraphValuesFromN(n, bands);
    std::cout << "n=" << n << std::endl;
    for (std::size_t band : bands) {
        std::cout << "band=" << band << " ";
        findMinM(n, STEP_SIZE, makeBandedGraph, 1, ORIENTABLE_LOW_BOUND, band);
    }
}

typedef std::pair<std::size_t, double> result_t;

static bool compareResults(const result_t& r1, const result_t& r2) {
    return r1.second < r2.second;
}

static result_t optimizeParamGivenN(std::size_t n, const HashGenFn& fn,
                                    std::size_t startParam, std::size_t stepBigParam, std::size_t stepSmallParam,
                                    bool forceMultiple) {
    std::vector<result_t> results;
    std::size_t param = startParam;
    std::cout << "n=" << n << std::endl;

    std::size_t start = 0, end = 0;
    for (std::size_t i = 0; i < 100; ++i, param += stepBigParam) {
        std::cout << "param=" << param << " ";
        double mn = findMinM(n, STEP_SIZE, fn, forceMultiple ? param : 1, ORIENTABLE_LOW_BOUND, param);
        results.push_back(std::make_pair(param, mn));
        if (results.size() > 1 && results[results.size() - 2].second < mn) {
            std::cout << "# Increased! Backtracking..." << std::endl;
            start = results.size() > 2 ? results[results.size() - 3].first : 5;
            end = param;
            break;
        }
    }

    if (start == 0 && end == 0) {
        std::cout << "# Could not find optimal value of param for n=" << n << std::endl;
    }

    for (param = start; param <= end; param += stepSmallParam) {
        std::cout << "param=" << param << " ";
        double mn = findMinM(n, STEP_SIZE, fn, forceMultiple ? param : 1, ORIENTABLE_LOW_BOUND, param);
        results.push_back(std::make_pair(param, mn));
    }

    const std::size_t MIN_K = 5;
    if (results.size() < MIN_K) {
        std::cout << "# Not enough data to decide optimal param" << std::endl;
        return std::make_pair(1, INFINITY);
    }

    std::partial_sort(results.begin(), results.begin() + MIN_K, results.end(), compareResults);
    double meanParam = 0;
    double meanMinimum = 0;
    for (std::size_t i = 0; i < MIN_K; ++i) meanParam += (MIN_K - i) * results[i].first;
    for (std::size_t i = 0; i < MIN_K; ++i) meanMinimum += results[i].second;
    meanParam /= (MIN_K * (MIN_K + 1) / 2);
    meanMinimum /= 5;
    std::cout << "n=" << n << " optim_param=" << meanParam << " optim_m/n=" << meanMinimum << std::endl;
    return std::make_pair((std::size_t) (meanParam + 0.5), meanMinimum);
}

static void generateOptimalEllGraph(void) {
    std::size_t n = 1000;
    for (std::size_t i = 2; i <= 6; ++i, n *= 10) {
        optimizeParamGivenN(n,     makeSegmentedGraph, 12, 20, 2, true);
        optimizeParamGivenN(n * 2, makeSegmentedGraph, 12, 20, 2, true);
        optimizeParamGivenN(n * 5, makeSegmentedGraph, 12, 20, 2, true);
    }
}

static void generateOptimalBandGraph(void) {
    std::size_t n = 1000;
    for (std::size_t i = 2; i <= 6; ++i, n *= 10) {
        optimizeParamGivenN(n,     makeBandedGraph, 6, 10, 2, false);
        optimizeParamGivenN(n * 2, makeBandedGraph, 6, 10, 2, false);
        optimizeParamGivenN(n * 5, makeBandedGraph, 6, 10, 2, false);
    }
}

int main() {
    std::cout.precision(10);
    fillPowCache();

    std::cout << "# Basic P vs m/n graph" << std::endl;
    generateBasicPvsMN();

    std::cout << "# Convergence graph (basic)" << std::endl;
    generateBasicConvergenceGraph();

    std::cout << "# Convergence graph (segmented)" << std::endl;
    generateSegmentConvergenceGraph();

    std::cout << "# Convergence graph (banded)" << std::endl;
    generateBandLimConvergenceGraph();

    std::cout << "# m/n vs l graph for fixed n" << std::endl;
    generateOptimalEllGraphForN(10000);
    generateOptimalEllGraphForN(30000);
    generateOptimalEllGraphForN(100000);
    
    std::cout << "# m/n vs band graph for fixed n" << std::endl;
    generateOptimalBandGraphForN(30000);
    generateOptimalBandGraphForN(100000);
    generateOptimalBandGraphForN(300000);

    // std::cout << "# optimal l given n" << std::endl;
    // optimizeParamGivenN(10000, makeSegmentedGraph, 12, 20, 2, true);

    // std::cout << "# optimal band given n" << std::endl;
    // optimizeParamGivenN(10000, makeBandedGraph, 6, 10, 2, false);

    std::cout << "# Optimal l graph" << std::endl;
    generateOptimalEllGraph();

    std::cout << "# Optimal band graph" << std::endl;
    generateOptimalBandGraph();
}
