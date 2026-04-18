#include <iostream>
#include <vector>
#include <chrono>
#include <unordered_map>
#include <numeric>
#include <algorithm>
#include <random>

extern "C" {
    #define HM_HASHMAP_IMPLEMENTATION
    #include "HashMap.h"
}

// Define the type helper for the benchmark
HM_DEFINE_TYPE(U64, uint64_t)

using namespace std::chrono;

void run_benchmark(uint32_t num_elements) {
    std::vector<uint64_t> keys(num_elements);
    std::iota(keys.begin(), keys.end(), 1000); // Sequential keys
    
    // Shuffle keys to test real-world hashing performance
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(keys.begin(), keys.end(), g);

    std::cout << "Benchmarking with " << num_elements << " elements...\n\n";

    // --- std::unordered_map ---
    {
        std::unordered_map<uint64_t, uint64_t> std_map;
        std_map.reserve(num_elements);

        auto start = high_resolution_clock::now();
        for (auto k : keys) std_map[k] = k;
        auto end = high_resolution_clock::now();
        std::cout << "std::unordered_map Insert: " << duration_cast<milliseconds>(end - start).count() << "ms\n";

        start = high_resolution_clock::now();
        volatile uint64_t sum = 0;
        for (auto k : keys) sum += std_map[k];
        end = high_resolution_clock::now();
        std::cout << "std::unordered_map Lookup: " << duration_cast<milliseconds>(end - start).count() << "ms\n";

        start = high_resolution_clock::now();
        for (auto k : keys) std_map.erase(k);
        end = high_resolution_clock::now();
        std::cout << "std::unordered_map Erase:  " << duration_cast<milliseconds>(end - start).count() << "ms\n\n";
    }

    // --- HashMap ---
    {
        HashMap my_map = HMCreate(num_elements, sizeof(uint64_t));

        auto start = high_resolution_clock::now();
        for (auto k : keys) HMInsertU64(&my_map, k, k);
        auto end = high_resolution_clock::now();
        std::cout << "HashMap Insert:     " << duration_cast<milliseconds>(end - start).count() << "ms\n";

        start = high_resolution_clock::now();
        volatile uint64_t sum = 0;
        for (auto k : keys) {
            uint64_t val;
            if (HMTryGetU64(&my_map, k, &val)) sum += val;
        }
        end = high_resolution_clock::now();
        std::cout << "HashMap Lookup:     " << duration_cast<milliseconds>(end - start).count() << "ms\n";

        start = high_resolution_clock::now();
        for (auto k : keys) HMErase(&my_map, k);
        end = high_resolution_clock::now();
        std::cout << "HashMap Erase:      " << duration_cast<milliseconds>(end - start).count() << "ms\n";

        HMDestroy(&my_map);
    }
}

int main() {
    run_benchmark(1'000'000);
    return 0;
}
