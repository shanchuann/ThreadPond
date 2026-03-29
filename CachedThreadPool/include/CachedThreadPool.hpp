#include "SyncQueue.hpp"
#include <functional>
#include <unordered_map>
#include <map>
#include <atomic>
#include <thread>
#include <memory>
#include <future>
#include <mutex>
#include <time.h>
#include <condition_variable>
#include <iostream>

#ifndef CACHEDTHREADPOOL_HPP
#define CACHEDTHREADPOOL_HPP

namespace shanchuan
{
    // 运行时日志开关：设置为 1 打印详细状态，0 关闭
#ifndef CTP_VERBOSE
#define CTP_VERBOSE 0
#endif

    static const time_t KeepAliveTimes = 60; // 线程空闲时间，单位为秒
    static const int InitThreadCount = 2;    // 初始线程数
    class CachedThreadPool
    {
    public:
        using Task = std::function<void(void)>;
    private:
        shanchuan::SyncQueue<Task> _task_queue;
        // 线程ID到线程对象的映射
        std::unordered_map<std::thread::id, std::shared_ptr<std::thread>> _thread_group;
        int MinThreadCount;                      // 最小线程数，保持线程池的基本处理能力
        int MaxThreadCount;                      // 最大线程数，限制线程池的资源占用
        mutable std::mutex _mutex;               // 保护线程组和活跃时间映射的互斥锁
        std::condition_variable _cv_thread_exit; // 线程是否结束
        std::atomic<bool> _is_running;           // 线程池是否正在运行
        std::atomic<int> _idle_thread_count;     // 当前空闲线程数
        std::once_flag _stop_once_flag;          // 确保线程池只被停止一次
        time_t _keep_alive_seconds;              // 空闲线程保活时间（秒），可配置用于测试

        void run_in_thread()
        {
            auto thread_id = std::this_thread::get_id();
            if (CTP_VERBOSE) std::cout << "[CachedThreadPool] thread start: " << thread_id << std::endl;
            time_t startTime = time(nullptr);
            while (_is_running)
            {
                Task task;
                int take_result = _task_queue.take(task);
                if (CTP_VERBOSE) std::cout << "[CachedThreadPool] thread " << thread_id << " take_result=" << take_result << " queue_empty=" << _task_queue.empty() << std::endl;
                if (_task_queue.empty())
                {
                    time_t now = time(nullptr);
                    std::unique_lock<std::mutex> lock(_mutex);
                    if (difftime(now, startTime) >= _keep_alive_seconds && _thread_group.size() > MinThreadCount)
                    {
                        if (CTP_VERBOSE) std::cout << "[CachedThreadPool] thread " << thread_id << " idle timeout -> detach and exit" << std::endl;
                        _thread_group.find(thread_id)->second->detach(); // 线程空闲超过 KeepAliveTimes，分离线程以便其自动退出
                        _thread_group.erase(thread_id);                  // 从线程组中移除该线程
                        _idle_thread_count--;                            // 空闲线程数减一
                        _cv_thread_exit.notify_all();                    // 通知其他线程
                        return;
                    }
                }
                if (take_result == 0 && _is_running)
                {
                    _idle_thread_count--;
                    if (CTP_VERBOSE) std::cout << "[CachedThreadPool] thread " << thread_id << " executing task" << std::endl;
                    task();
                    if (CTP_VERBOSE) std::cout << "[CachedThreadPool] thread " << thread_id << " finished task" << std::endl;
                    _idle_thread_count++;
                    startTime = time(nullptr); // 更新线程的最后活跃时间
                }
                else if (take_result == QUEUE_STOPPED)
                {
                    if (CTP_VERBOSE) std::cout << "[CachedThreadPool] thread " << thread_id << " detected queue stopped -> exit" << std::endl;
                    break; // 线程池正在停止，退出线程
                }

                // 检查线程是否空闲超过 KeepAliveTimes，并且当前线程数超过最小线程数
                auto now = std::chrono::steady_clock::now();
                if (_idle_thread_count > 0 && std::chrono::duration_cast<std::chrono::seconds>(now - std::chrono::steady_clock::time_point(std::chrono::steady_clock::now() - std::chrono::seconds(_keep_alive_seconds))).count() >= _keep_alive_seconds)
                {
                    std::lock_guard<std::mutex> lock(_mutex);
                    if (_thread_group.size() > MinThreadCount)
                    {
                        // 让当前线程退出
                        _cv_thread_exit.notify_all();
                        break;
                    }
                }
            }
            // 线程退出前从线程组中移除自己
            std::lock_guard<std::mutex> lock(_mutex);
            auto it = _thread_group.find(std::this_thread::get_id());
            if (it != _thread_group.end()) {
                if (CTP_VERBOSE) std::cout << "[CachedThreadPool] thread " << std::this_thread::get_id() << " exiting and removing self" << std::endl;
                if (it->second && it->second->joinable()) {
                    it->second->detach(); // 在销毁前 detach，避免在当前线程中析构 joinable std::thread 导致 std::terminate
                }
                _thread_group.erase(it);
            }
        }
        void start(int num_threads)
        {
            _is_running = true;
            for (int i = 0; i < num_threads; ++i)
            {
                auto thread = std::make_shared<std::thread>(&shanchuan::CachedThreadPool::run_in_thread, this);
                _thread_group[thread->get_id()] = thread;
                _idle_thread_count++;
                if (CTP_VERBOSE) std::cout << "[CachedThreadPool] start(): created thread " << thread->get_id() << " (total=" << _thread_group.size() << ")" << std::endl;
            }
        }
        void stop_thread_group()
        {
            // 先唤醒所有正在等待的 take()，让线程尽快退出，避免 std::thread 在可 join 状态下被析构
            if (CTP_VERBOSE) std::cout << "[CachedThreadPool] stop_thread_group(): begin" << std::endl;
            _task_queue.force_stop();
            _is_running = false;
            MinThreadCount = 0; // 设置最小线程数为0，允许所有线程退出
            std::unique_lock<std::mutex> lock(_mutex);
            while(_thread_group.size() > 0)
            {
                _cv_thread_exit.wait_for(lock, std::chrono::seconds(1)); // 等待所有线程退出或超时
                for (auto &pair : _thread_group)
                {
                    auto thr = pair.second;
                    if (thr && thr->joinable())
                    {
                        if (CTP_VERBOSE) std::cout << "[CachedThreadPool] stop_thread_group(): joining thread " << pair.first << std::endl;
                        thr->join(); // 等待线程退出
                        if (CTP_VERBOSE) std::cout << "[CachedThreadPool] stop_thread_group(): joined thread " << pair.first << std::endl;
                    }
                }
                // 清理已退出的线程对象（被 run_in_thread 移除的 id 已经从 map 删除）
                for (auto it = _thread_group.begin(); it != _thread_group.end(); ) {
                    if (!it->second || !it->second->joinable()) {
                        if (CTP_VERBOSE) std::cout << "[CachedThreadPool] stop_thread_group(): erasing thread entry " << it->first << std::endl;
                        it = _thread_group.erase(it);
                    } else ++it;
                }
            }
            if (CTP_VERBOSE) std::cout << "[CachedThreadPool] stop_thread_group(): end" << std::endl;
        }
        void add_thread()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            if (_idle_thread_count == 0 && _thread_group.size() < MaxThreadCount)
            {
                auto thread = std::make_shared<std::thread>(&shanchuan::CachedThreadPool::run_in_thread, this);
                _thread_group[thread->get_id()] = thread;
                _idle_thread_count++;
                if (CTP_VERBOSE) std::cout << "[CachedThreadPool] add_thread(): created thread " << thread->get_id() << " (total=" << _thread_group.size() << ")" << std::endl;
            }
        }

    public:
        CachedThreadPool(int min_threads = InitThreadCount, int max_threads = std::thread::hardware_concurrency(), int keep_alive_seconds = KeepAliveTimes) : _task_queue(MaxTaskCount, 1), MinThreadCount(min_threads), MaxThreadCount(max_threads), _is_running(false), _idle_thread_count(0), _keep_alive_seconds(keep_alive_seconds)
        {
            start(MinThreadCount);
        }
        ~CachedThreadPool()
        {
            if (_is_running) stop();
        }
        void stop()
        {
            if (CTP_VERBOSE) std::cout << "[CachedThreadPool] stop(): called" << std::endl;
            std::call_once(_stop_once_flag, [this]() { stop_thread_group(); });
        }
        void add_task(const Task &task)
        {
            if (_task_queue.put(task) != 0)
            {
                if (CTP_VERBOSE) std::cout << "[CachedThreadPool] add_task(): failed to put task (maybe full or stopping)" << std::endl;
                throw std::runtime_error("Failed to add task to the queue, it may be full or the thread pool is stopping.");
                task(); // 直接执行任务，避免丢失
            }
            else
            {
                add_thread(); // 尝试添加线程以处理增加的任务
            }
        }
        void add_task(Task &&task)
        {
            if (_task_queue.put(std::forward<Task>(task)) != 0)
            {
                if (CTP_VERBOSE) std::cout << "[CachedThreadPool] add_task(rvalue): failed to put task (maybe full or stopping)" << std::endl;
                throw std::runtime_error("Failed to add task to the queue, it may be full or the thread pool is stopping.");
                task(); // 直接执行任务，避免丢失
            }
            else
            {
                add_thread(); // 尝试添加线程以处理增加的任务
            }
        }
        template <typename F, typename... Args>
        auto submit(F &&f, Args &&...args)
        {
            using result_type = std::invoke_result_t<F, Args...>;
            auto task = std::make_shared<std::packaged_task<result_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
            auto future = task->get_future();
            Task wrapper = [task]() mutable { (*task)(); };
            // 尝试提交包装后的任务到队列，队列已满或正在停止时，直接执行任务以避免丢失
            if (_task_queue.put(wrapper) != 0)
            {
                if (CTP_VERBOSE) std::cout << "[CachedThreadPool] submit(): queue full or stopping, executing task in caller thread" << std::endl;
                (*task)();
            }
            return future;
        }
        int get_thread_count() const
        {
            std::lock_guard<std::mutex> lock(_mutex);
            return _thread_group.size();
        }
    };
}
#endif // CACHEDTHREADPOOL_HPP