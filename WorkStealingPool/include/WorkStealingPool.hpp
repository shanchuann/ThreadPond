#include <functional>
#include <vector>
#include <atomic>
#include <thread>
#include <memory>
#include <future>
#include <type_traits>
#include "SyncQueue.hpp"

#ifndef WORKSTEALINGPOOL_HPP
#define WORKSTEALINGPOOL_HPP

namespace shanchuan
{
    class WorkStealingPool
    {
    public:
        using Task = std::function<void(void)>;

    private:
        std::vector<std::shared_ptr<std::thread>> _thread_group;
        std::atomic<bool> _is_running;          // 线程池是否正在运行
        std::once_flag _stop_once_flag;         // 确保线程池只被停止一次
        size_t _num_threads;                    // 线程池中的线程数量
        shanchuan::SyncQueue<Task> _task_queue; // 任务队列，直接作为成员
        // 统计信息
        std::unique_ptr<std::atomic<int>[]> _processed_count; // 每个线程处理的任务数
        std::unique_ptr<std::atomic<int>[]> _stolen_count;    // 每个线程窃取到的任务数
        std::atomic<int> _total_stolen{0};                    // 总共被窃取的任务数
        int thread_index() const
        {
            static size_t index = 0;
            return index++ % _num_threads;
        }
        void start(int num_threads)
        {
            _is_running = true;
            // 初始化统计容器（使用原子数组）
            _processed_count = std::make_unique<std::atomic<int>[]>(num_threads);
            _stolen_count = std::make_unique<std::atomic<int>[]>(num_threads);
            for (int j = 0; j < num_threads; ++j) { _processed_count[j].store(0, std::memory_order_relaxed); _stolen_count[j].store(0, std::memory_order_relaxed); }
            _total_stolen.store(0, std::memory_order_relaxed);
            for (int i = 0; i < num_threads; ++i)
            {
                _thread_group.push_back(std::make_shared<std::thread>(&shanchuan::WorkStealingPool::run_in_thread, this, i));
            }
        }
        void run_in_thread(int thread_id)
        {
            while (_is_running)
            {
                std::deque<Task> taskque;
                if (_task_queue.take(taskque, thread_id) == 0)
                {
                    int n = static_cast<int>(taskque.size());
                    for (auto &task : taskque) if (task) task();
                    _processed_count[thread_id].fetch_add(n, std::memory_order_relaxed);
                }
                else
                {
                    int i = thread_index();
                    if (i != thread_id && _task_queue.take(taskque, i) == 0)
                    {
                        int n = static_cast<int>(taskque.size());
                        for (auto &task : taskque) if (task) task();
                        _processed_count[thread_id].fetch_add(n, std::memory_order_relaxed);
                        _stolen_count[thread_id].fetch_add(n, std::memory_order_relaxed);
                        _total_stolen.fetch_add(n, std::memory_order_relaxed);
                    }
                }
            }
        }
        void stop_thread_group()
        {
            _task_queue.force_stop();
            _is_running = false;
            for (auto &thread : _thread_group)
                if (thread->joinable()) thread->join();
            _thread_group.clear();
        }

    public:
        WorkStealingPool(const size_t num_threads = 8, const size_t queue_size = 200)
            : _is_running(false), _stop_once_flag(), _num_threads(num_threads), _task_queue(static_cast<int>((queue_size + num_threads - 1) / num_threads), queue_size)
        {
            start(_num_threads);
        }
        ~WorkStealingPool()
        {
            if (_is_running)
                stop();
        }
        void stop()
        {
            std::call_once(_stop_once_flag, [this]()
                           { stop_thread_group(); });
        }
        // 导出统计信息的快照（线程安全读取）
        void collect_stats(std::vector<int> &processed, std::vector<int> &stolen, int &total) const
        {
            processed.clear();
            stolen.clear();
            // arrays sized to _num_threads
            for (size_t i = 0; i < _num_threads; ++i)
            {
                processed.push_back(_processed_count[i].load(std::memory_order_relaxed));
                stolen.push_back(_stolen_count[i].load(std::memory_order_relaxed));
            }
            total = _total_stolen.load(std::memory_order_relaxed);
        }
        void add_task(const Task &task)
        {
            if (_task_queue.put(task, thread_index()) != 0)
            {
                throw std::runtime_error("Failed to add task to the queue, it may be full or the thread pool is stopping.");
                task(); // 直接执行任务，避免丢失
            }
        }
        void add_task(Task &&task)
        {
            if (_task_queue.put(std::forward<Task>(task), thread_index()) != 0)
            {
                throw std::runtime_error("Failed to add task to the queue, it may be full or the thread pool is stopping.");
                task(); // 直接执行任务，避免丢失
            }
        }
        template <typename F, typename... Args>
        auto submit(F &&f, Args &&...args)
        {
            using result_type = std::invoke_result_t<F, Args...>;
            auto task = std::make_shared<std::packaged_task<result_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
            auto future = task->get_future();
            Task wrapper = [task]() mutable
            { (*task)(); };
            // 尝试提交包装后的任务到队列，队列已满或正在停止时，直接执行任务以避免丢失
            if (_task_queue.put(wrapper, thread_index()) != OK)
            {
                (*task)();
            }
            return future;
        }
        void print(){
            _task_queue.print_task_info();
        }
    };
}
#endif // WORKSTEALINGPOOL_HPP