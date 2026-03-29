#include "CachedThreadPool.hpp"
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>
#include <functional>
#include <future>

#define USE_FIXEDTEST 0
#if USE_FIXEDTEST

using namespace shanchuan;

int main()
{
    using clock = std::chrono::high_resolution_clock;

    struct Result { double submit_ms; double total_ms; int final_counter; };

    auto run_workload = [&](bool use_pool)->Result {
        const int TASKS = 200;               // number of tasks
        const int TASK_MS = 50;              // simulated work per task in ms

        std::atomic<int> counter{0};
        std::vector<std::future<int>> futures;

        clock::time_point t_total_start = clock::now();
        clock::time_point t_submit_start = clock::now();

        if (use_pool) {
            // create pool with reasonable max threads, ensure verbose off in header for fair test
            CachedThreadPool pool(2, std::max(2u, std::thread::hardware_concurrency()));

            t_submit_start = clock::now();
            for (int i = 0; i < TASKS; ++i) {
                pool.add_task([&counter, TASK_MS]() {
                    std::this_thread::sleep_for(std::chrono::milliseconds(TASK_MS));
                    ++counter;
                });
            }

            clock::time_point t_submit_end = clock::now();

            // wait until tasks complete or timeout
            for (int i = 0; i < 2000 && counter.load() < TASKS; ++i) std::this_thread::sleep_for(std::chrono::milliseconds(10));

            for (auto &f : futures) { try { f.get(); } catch(...){} }

            clock::time_point t_end = clock::now();
            int final = counter.load();
            pool.stop();
            return Result{std::chrono::duration<double, std::milli>(t_submit_end - t_submit_start).count(), std::chrono::duration<double, std::milli>(t_end - t_total_start).count(), final};
        } else {
            // direct execution in caller thread
            t_submit_start = clock::now();
            for (int i = 0; i < TASKS; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(TASK_MS));
                ++counter;
            }
            clock::time_point t_submit_end = clock::now();
            clock::time_point t_end = clock::now();
            int final = counter.load();
            return Result{std::chrono::duration<double, std::milli>(t_submit_end - t_submit_start).count(), std::chrono::duration<double, std::milli>(t_end - t_total_start).count(), final};
        }
    };

    // Run with pool enabled then disabled, identical workload
    Result r1 = run_workload(true);
    Result r2 = run_workload(false);

    std::cout << "RESULT (pool enabled): submit=" << r1.submit_ms << " ms, total=" << r1.total_ms << " ms, counter=" << r1.final_counter << "\n";
    std::cout << "RESULT (pool disabled): submit=" << r2.submit_ms << " ms, total=" << r2.total_ms << " ms, counter=" << r2.final_counter << "\n";
    std::cout << "DIFF total(enabled - disabled) = " << (r1.total_ms - r2.total_ms) << " ms\n";

    return 0;
}
#endif