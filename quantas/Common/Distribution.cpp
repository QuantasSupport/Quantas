#include "Distribution.hpp"

namespace quantas
{
    std::hash<std::thread::id> _hasher;
    
    thread_local default_random_engine RANDOM_GENERATOR =
        default_random_engine(static_cast<int>(time(nullptr))+_hasher(std::this_thread::get_id()));
    
    int uniformInt(const int min, const int max)
    {
        std::uniform_int_distribution<int> distribution(min, max);
        return distribution(RANDOM_GENERATOR);
    }

    int randMod(const int exclusiveMax)
    {
        std::uniform_int_distribution<int> distribution(0, exclusiveMax - 1);
        return distribution(RANDOM_GENERATOR);
    }
}
