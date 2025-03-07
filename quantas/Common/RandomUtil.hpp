#ifndef RANDOM_UTIL_HPP
#define RANDOM_UTIL_HPP

#include <random>
#include <thread>
#include <ctime>
#include <stdexcept>
#include <string>

namespace quantas {

//
// 1) A single thread_local function that gives us the engine
//
inline std::mt19937& threadLocalEngine() {
    // We combine the time and the hashed thread ID to get a seed unique to each thread
    static thread_local std::mt19937 engine(
        static_cast<unsigned>(std::time(nullptr))
        + static_cast<unsigned>(std::hash<std::thread::id>{}(std::this_thread::get_id()))
    );
    return engine;
}

//
// 2) Uniform integer in [min, max]
//
inline int uniformInt(int minVal, int maxVal) {
    if (minVal > maxVal) {
        throw std::invalid_argument("uniformInt: minVal > maxVal");
    }
    std::uniform_int_distribution<int> dist(minVal, maxVal);
    return dist(threadLocalEngine());
}

//
// 3) Uniform real in [min, max)
//
inline double uniformReal(double minVal, double maxVal) {
    if (minVal > maxVal) {
        throw std::invalid_argument("uniformReal: minVal > maxVal");
    }
    std::uniform_real_distribution<double> dist(minVal, maxVal);
    return dist(threadLocalEngine());
}

//
// 4) randMod(exclusiveMax) -> returns integer in [0, exclusiveMax-1]
//
inline int randMod(int exclusiveMax) {
    if (exclusiveMax <= 0) {
        throw std::invalid_argument(
            "randMod: exclusiveMax must be > 0, received: " + std::to_string(exclusiveMax)
        );
    }
    // Just reuse uniformInt
    return uniformInt(0, exclusiveMax - 1);
}

//
// 5) trueWithProbability(p) -> returns true with probability p
//
inline bool trueWithProbability(double p) {
    if (p <= 0.0) {
        return false;
    } else if (p >= 1.0) {
        return true;
    } else {
        return (uniformReal(0.0, 1.0) < p);
    }
}

inline int poissonInt(double lambda) {
    if (lambda <= 0.0) {
        throw std::invalid_argument(
            "poissonInt: mean (lambda) must be > 0, received: " + std::to_string(lambda)
        );
    }
    std::poisson_distribution<int> dist(lambda);
    return dist(threadLocalEngine());
}

} // namespace quantas

#endif // RANDOM_UTIL_HPP