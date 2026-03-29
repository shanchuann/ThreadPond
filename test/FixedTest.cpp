#include "FixedThreadPool.hpp"
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>
#include <functional>
#include <future>

#define USE_FIXEDTEST 0
#if USE_FIXEDTEST

#ifndef USE_FIXED_THREADPOOL
#define USE_FIXED_THREADPOOL 0
#endif

using namespace shanchuan;

// 不同类型的可调用对象
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
    std::atomic<int> counter{0};
    const int N = 20;
    using clock = std::chrono::high_resolution_clock;
    clock::time_point t_total_start = clock::now();
    clock::time_point t_submit_start;
    clock::time_point t_submit_end;
    clock::time_point t_end;

#if USE_FIXED_THREADPOOL
    FixedThreadPool pool(std::max(1u, std::thread::hardware_concurrency()));

    t_submit_start = clock::now();
    // 1) 自由函数（绑定参数）
    for (int i = 0; i < N; ++i)
    {
        pool.add_task(std::bind(free_function, 1, std::ref(counter)));
    }

    // 2) 无捕获 lambda
    for (int i = 0; i < N; ++i)
    {
        pool.add_task([&counter]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            ++counter;
        });
    }

    // 3) 捕获 lambda
    int add = 2;
    for (int i = 0; i < N; ++i)
    {
        pool.add_task([&counter, add]() mutable {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            counter += add;
        });
    }

    // 4) 可调用对象
    Functor f;
    for (int i = 0; i < N; ++i)
    {
        pool.add_task(std::bind(f, std::ref(counter)));
    }

    // 5) 成员函数 + submit（有返回值）
    Obj obj;
    std::vector<std::future<int>> futures;
    for (int i = 1; i <= 5; ++i)
    {
        futures.push_back(pool.submit(&Obj::multiply, &obj, i, i + 1));
    }
    t_submit_end = clock::now();

#else
    // 不使用线程池：在主线程直接执行相同的任务
    t_submit_start = clock::now();
    for (int i = 0; i < N; ++i) free_function(1, counter);
    for (int i = 0; i < N; ++i) { std::this_thread::sleep_for(std::chrono::milliseconds(5)); ++counter; }
    int add = 2;
    for (int i = 0; i < N; ++i) counter += add;
    Functor f;
    for (int i = 0; i < N; ++i) f(counter);
    Obj obj;
    std::vector<std::future<int>> futures;
    for (int i = 1; i <= 5; ++i)
    {
        // 直接调用并用 std::async 获取 future（模拟 submit）
        futures.push_back(std::async(std::launch::deferred, &Obj::multiply, &obj, i, i + 1));
    }
    t_submit_end = clock::now();
#endif

    // 等待并收集 submit 的结果
    int sum_mul = 0;
    for (auto &fu : futures)
    {
        try {
            sum_mul += fu.get();
        } catch (...) {}
    }

    // 给线程池一些时间让任务完成（如果使用线程池的话，析构会等待）
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    t_end = clock::now();

    using ms = std::chrono::duration<double, std::milli>;
    std::cout << "Submit time: " << ms(t_submit_end - t_submit_start).count() << " ms\n";
    std::cout << "Total elapsed: " << ms(t_end - t_total_start).count() << " ms\n";
    std::cout << "Final counter = " << counter << "\n";
    std::cout << "Sum of multiply futures = " << sum_mul << "\n";

    return 0;
}
#endif