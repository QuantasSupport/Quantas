#include "Distribution.hpp"

namespace quantas {
std::hash<std::thread::id> _hasher;

thread_local default_random_engine RANDOM_GENERATOR = default_random_engine(
    static_cast<int>(clock()) + _hasher(std::this_thread::get_id())
);

int uniformInt(const int min, const int max) {
    static thread_local std::mt19937 generator(
        clock() + _hasher(std::this_thread::get_id())
    );
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(generator);
}

int randMod(const int exclusiveMax) {
    static thread_local std::mt19937 generator(
        clock() + _hasher(std::this_thread::get_id())
    );
    std::uniform_int_distribution<int> distribution(0, exclusiveMax - 1);
    return distribution(generator);
}

bool oneInXChance(const int x) { return uniformInt(1, x) == 1; }

bool trueWithProbability(const double p) {
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static thread_local std::mt19937 generator(
        clock() + _hasher(std::this_thread::get_id())
    );
    return distribution(generator) < p;
}
} // namespace quantas
