#include "../WorkStealingPool/WorkStealingPool.hpp"
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>
#include <functional>
#include <future>

#define USE_WORKSTEALING_TEST 0
#if USE_WORKSTEALING_TEST
using namespace shanchuan;

// 与 FixedTest 相似的多种可调用对象测试，同时尝试制造任务不均衡以触发窃取
void free_function(int n, std::atomic<int> &counter)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    counter += n;
}

struct Functor
{
    void operator()(std::atomic<int> &counter)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        ++counter;
    }
};

struct Obj
{
    int multiply(int a, int b)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        return a * b;
    }
};

int main()
{
    using clock = std::chrono::high_resolution_clock;

    struct RunResult {
        double submit_ms;
        double total_ms;
        int final_counter;
        int sum_mul;
        std::vector<int> processed_per_thread;
        std::vector<int> stolen_per_thread;
        int total_stolen;
    };

    auto run_workload = [&](bool use_pool, unsigned num_threads)->RunResult {
        const int TASKS = 200;
        const int TASK_MS = 5;
        const int LONG_TASKS = 8;
        const int LONG_TASK_MS = 80;

        std::atomic<int> counter{0};
        std::vector<std::future<int>> futures;

        clock::time_point t_total_start = clock::now();
        clock::time_point t_submit_start = clock::now();
        clock::time_point t_submit_end;
        clock::time_point t_end;

        if (use_pool) {
            WorkStealingPool pool(std::max(2u, num_threads));

            // prewarm: submit one tiny task per thread to ensure workers are started
            for (unsigned i = 0; i < std::max(2u, num_threads); ++i)
                pool.add_task([](){ std::this_thread::sleep_for(std::chrono::milliseconds(1)); });
            std::this_thread::sleep_for(std::chrono::milliseconds(20));

            t_submit_start = clock::now();

            // 1) free functions
            for (int i = 0; i < TASKS/4; ++i)
                pool.add_task(std::bind(free_function, 1, std::ref(counter)));

            // 2) short lambdas
            for (int i = 0; i < TASKS/4; ++i)
                pool.add_task([&counter, TASK_MS]() { std::this_thread::sleep_for(std::chrono::milliseconds(TASK_MS)); ++counter; });

            // 3) a few long tasks that spawn small tasks
            for (int i = 0; i < LONG_TASKS; ++i)
            {
                pool.add_task([&counter, &pool, LONG_TASK_MS]() {
                    std::this_thread::sleep_for(std::chrono::milliseconds(LONG_TASK_MS));
                    ++counter;
                    for (int k = 0; k < 4; ++k)
                        pool.add_task([&counter]() { std::this_thread::sleep_for(std::chrono::milliseconds(10)); ++counter; });
                });
            }

            // 4) Functor tasks
            Functor f;
            for (int i = 0; i < TASKS/8; ++i)
                pool.add_task(std::bind(f, std::ref(counter)));

            // 5) submit futures
            Obj obj;
            for (int i = 1; i <= 5; ++i)
                futures.push_back(pool.submit(&Obj::multiply, &obj, i, i + 1));

            t_submit_end = clock::now();

            // wait a bit for tasks to finish
            for (int i = 0; i < 500 && counter.load() < TASKS + LONG_TASKS*4; ++i)
                std::this_thread::sleep_for(std::chrono::milliseconds(10));

            for (auto &fu : futures) { try { (void)fu.get(); } catch(...) {} }

            t_end = clock::now();
            int sum_mul = 0; // collect futures results
            try { for (auto &fu : futures) { sum_mul += fu.get(); } } catch(...) {}

            // collect internal stats
            std::vector<int> processed;
            std::vector<int> stolen;
            int total_stolen = 0;
            pool.collect_stats(processed, stolen, total_stolen);

            return RunResult{std::chrono::duration<double, std::milli>(t_submit_end - t_submit_start).count(),
                             std::chrono::duration<double, std::milli>(t_end - t_total_start).count(),
                             counter.load(), sum_mul, processed, stolen, total_stolen};
        } else {
            // direct execution
            t_submit_start = clock::now();

            for (int i = 0; i < TASKS/4; ++i) free_function(1, counter);
            for (int i = 0; i < TASKS/4; ++i) { std::this_thread::sleep_for(std::chrono::milliseconds(TASK_MS)); ++counter; }
            for (int i = 0; i < LONG_TASKS; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(LONG_TASK_MS)); ++counter;
                for (int k = 0; k < 4; ++k) { std::this_thread::sleep_for(std::chrono::milliseconds(10)); ++counter; }
            }
            Functor f;
            for (int i = 0; i < TASKS/8; ++i) f(counter);
            Obj obj;
            std::vector<int> local_results;
            for (int i = 1; i <= 5; ++i) local_results.push_back(obj.multiply(i, i + 1));

            t_submit_end = clock::now();
            t_end = clock::now();
            int sum_mul = 0; for (int v : local_results) sum_mul += v;
            return RunResult{std::chrono::duration<double, std::milli>(t_submit_end - t_submit_start).count(),
                             std::chrono::duration<double, std::milli>(t_end - t_total_start).count(),
                             counter.load(), sum_mul, {}, {}, 0};
        }
    };

    unsigned hw = std::max(2u, std::thread::hardware_concurrency());
    std::cout << "WorkStealingPool Benchmark\n";
    std::cout << "Hardware threads: " << hw << "\n";
    std::cout << "Tasks: 200, short task 5ms, long tasks 8 x 80ms + 4 x 10ms spawn\n";

    // try multiple thread counts
    std::vector<unsigned> tests = {2, 4, 8, hw};
    // unique and <= hw
    std::sort(tests.begin(), tests.end());
    tests.erase(std::unique(tests.begin(), tests.end()), tests.end());
    for (unsigned t : tests) {
        if (t > hw) continue;
        std::cout << "\n=== Threads = " << t << " ===\n";
        auto rp = run_workload(true, t);
        auto rd = run_workload(false, t);

        std::cout << "Pool: submit=" << rp.submit_ms << " ms, total=" << rp.total_ms << " ms, final_counter=" << rp.final_counter << ", sum_mul=" << rp.sum_mul << "\n";
        std::cout << "Direct: submit=" << rd.submit_ms << " ms, total=" << rd.total_ms << " ms, final_counter=" << rd.final_counter << ", sum_mul=" << rd.sum_mul << "\n";
        std::cout << "Diff (pool - direct) total = " << (rp.total_ms - rd.total_ms) << " ms\n";

        if (!rp.processed_per_thread.empty()) {
            std::cout << "Per-thread processed:\n";
            for (size_t i = 0; i < rp.processed_per_thread.size(); ++i)
                std::cout << "  thread[" << i << "]=" << rp.processed_per_thread[i] << ", stolen=" << rp.stolen_per_thread[i] << "\n";
            std::cout << "Total stolen tasks: " << rp.total_stolen << "\n";
        }
    }

    return 0;
}
#endif