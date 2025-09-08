#include <iostream>
#include <chrono>
#include <sstream>
#include <format>


#include "hwy/base.h"           // HWY_ASSERT
#include <hwy/auto_tune.h>
#define HWY_TEST_STANDALONE 1
#include "hwy/tests/test_util-inl.h"

namespace hwy{
namespace my_test{
    // Returns random floating-point number in [-8, 8).
    static double Random(RandomState& rng) {
        const int32_t bits = static_cast<int32_t>(Random32(&rng)) & 1023;
        return (bits - 512) / 64.0;
    }

    double do_auto_tune_run(const size_t num, const size_t max_reps){
        // Gaussian distribution with additive+multiplicative noise.
        RandomState rng;
        double mean_cost = 0.0;
        for (size_t rep = 0; rep < max_reps; ++rep) {
            CostDistribution cd;
            for (size_t i = 0; i < num; ++i) {
                // Central limit theorem: sum of independent random is Gaussian.
                double sum = 500.0;
                for (size_t sum_idx = 0; sum_idx < 100; ++sum_idx) sum += Random(rng);

                // 16% noise: mostly additive, some lucky shots.
                const uint32_t r = Random32(&rng);
                if (r < (1u << 28)) {
                    static constexpr double kPowers[4] = {0.0, 1E3, 1E4, 1E5};
                    static constexpr double kMul[4] = {0.50, 0.75, 0.85, 0.90};
                    if (r & 3) {  // 75% chance of large additive noise
                    sum += kPowers[r & 3];
                    } else {  // 25% chance of small multiplicative reduction
                    sum *= kMul[(r >> 2) & 3];
                    }
                }
                cd.Notify(sum);
            }
            mean_cost += cd.EstimateCost();
        }
        return mean_cost / max_reps;
    }
}
}


int main(int argc, char* argv[]){
    size_t n_samples = 14;
    if (argc > 1){
        std::stringstream ss(argv[1]);
        ss >> n_samples;
    }

    size_t n_runs = 20000;
    if(argc == 3){
        std::stringstream ss(argv[2]);
        ss >> n_runs;
    }
    
    std::cout << "number of samples: " << n_samples << std::endl;
    std::cout << "Number of repetitions: " << n_runs << std::endl;

    auto t_start = std::chrono::steady_clock::now();
    double avg_cost = hwy::my_test::do_auto_tune_run(n_samples, n_runs);
    auto t_end = std::chrono::steady_clock::now();
    auto total = std::chrono::floor<std::chrono::microseconds>(t_end - t_start);
    auto time_per_rep = std::chrono::floor<std::chrono::nanoseconds>((t_end - t_start)/n_runs);

    std::cout << std::format("Average cost: {}", avg_cost) << std::endl;
    
    std::cout << std::format("Total time: {0:%T}", total) << std::endl;
    std::cout << std::format("Runtime per call: {0:%T}", time_per_rep) << std::endl;
}