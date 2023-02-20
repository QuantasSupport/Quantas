#include <iostream>
#include <thread>
#include <vector>
#include <cassert>
#include "../Common/Distribution.hpp"

void getRandomInts(std::vector<int> &randomInts, int howMany)
{
    for (int i = 0; i < howMany; i++)
    {
        randomInts.push_back(quantas::uniformInt(0, 1 << 16));
    }
}

int main()
{
    const int RandIntCount = 10;
    const int ThreadCount = 10;

    std::vector<std::vector<int>> results(ThreadCount);
    std::vector<std::thread> threads(ThreadCount);
    for (int j = 0; j < ThreadCount; j++)
    {
        threads[j] = std::thread(getRandomInts, std::ref(results[j]), RandIntCount);
    }

    for (auto &t : threads)
    {
        t.join();
    }

    for (auto &vecForThread : results)
    {
        for (auto &numInVec : vecForThread)
        {
            std::cout << numInVec << " ";
        }
        std::cout << std::endl;
    }

    for (int k = 0; k < RandIntCount; k++)
    {
        for (int threadIndex = 1; threadIndex < ThreadCount; threadIndex++)
        {
            assert(results[0][k] != results[threadIndex][k]);
        }
    }

    return 0;
}
