#include <functional>
#include <list>
#include <atomic>
#include <thread>
#include <memory>
#include "SyncQueue.hpp"

#ifndef FIXEDTHREADPOOL_HPP
#define FIXEDTHREADPOOL_HPP

namespace shanchuan
{
    class FixedThreadPool
    {
    public:
        using Task = std::function<void(void)>;

    private:
        std::list<std::shared_ptr<std::thread>> _thread_group;
        shanchuan::SyncQueue<Task> _task_queue;
        std::atomic<bool> _is_running;  // 线程池是否正在运行
        std::once_flag _stop_once_flag; // 确保线程池只被停止一次

        void start(int num_threads)
        {
            _is_running = true;
            for (int i = 0; i < num_threads; ++i)
            {
                _thread_group.push_back(std::make_shared<std::thread>(&shanchuan::FixedThreadPool::run_in_thread, this));
            }
        }
        void run_in_thread()
        {
            while (_is_running)
            {
                Task task;
                try
                {
                    _task_queue.take(&task);
                }
                catch (const std::runtime_error &e)
                {
                    // 线程池正在停止，退出线程
                    throw std::runtime_error("ThreadPool is stopping, exiting thread.");
                    break;
                }
                if (task)
                {
                    task();
                }
            }
        }
        void stop_thread_group()
        {
            _task_queue.wait_stop();
            _is_running = false;
            for (auto &thread : _thread_group)
            {
                if (thread->joinable())
                {
                    thread->join();
                }
            }
            _thread_group.clear();
        }

    public:
        FixedThreadPool(int num_threads = std::thread::hardware_concurrency()) : _task_queue(MaxTaskCount), _is_running(false)
        {
            start(num_threads);
        }
        ~FixedThreadPool()
        {
            stop();
        }
        void stop()
        {
            std::call_once(_stop_once_flag, [this]()
                           { stop_thread_group(); });
        }
        void add_task(const Task &task)
        {
            if (!_is_running)
            {
                throw std::runtime_error("ThreadPool is not running, cannot add new tasks.");
            }
            _task_queue.put(task);
        }
        void add_task(Task &&task)
        {
            if (!_is_running)
            {
                throw std::runtime_error("ThreadPool is not running, cannot add new tasks.");
            }
            _task_queue.put(std::forward<Task>(task));
        }
    };

}

#endif // FIXEDTHREADPOOL_HPP