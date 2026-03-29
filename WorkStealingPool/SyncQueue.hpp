#include <mutex>
#include <deque>
#include <vector>
#include <iostream>
#include <condition_variable>

#ifndef SYNCQUEUE_HPP
#define SYNCQUEUE_HPP

#define OK 0
#define QUEUE_FULL 1
#define QUEUE_STOPPED 2
static const int MaxTaskCount = 100;

namespace shanchuan
{
    template <typename T>
    class SyncQueue
    {
    private:
        std::vector<std::deque<T>> _task_queue; // 任务队列，使用双端队列实现
        size_t _bucket_size;                    // 任务队列的桶大小，控制每个桶中任务的数量，避免单个桶过大导致性能问题
        size_t _max_size;                       // 任务队列的最大容量 vector[0] -> deque<T> size
        mutable std::mutex _mutex;
        std::condition_variable _cv_not_empty;  // 当队列不为空时通知消费者线程
        std::condition_variable _cv_not_full;   // 当队列不满时通知生产者线程

        bool _stop_flag;       // true表示线程池正在停止，false表示线程池正常运行
        int _wait_timeout = 1; // 等待超时时间，单位为秒

        bool is_full(const size_t &bucket_index) const
        {
            bool full = _task_queue[bucket_index].size() >= _bucket_size;
            return full;
        }
        bool is_empty(const size_t &bucket_index) const
        {
            bool empty = _task_queue[bucket_index].empty();
            return empty;
        }
        template <typename F>
        int add(F &&task, size_t bucket_index)
        {
            std::unique_lock<std::mutex> lock(_mutex);

            // 生产者线程在添加任务时，如果队列已满，则等待直到队列不满或线程池正在停止，等待时设置超时时间，避免死锁
            bool tag = _cv_not_full.wait_for(lock, std::chrono::seconds(_wait_timeout), [=]()
                                             { return !is_full(bucket_index) || _stop_flag; });
            if (!tag)
                return QUEUE_FULL;
            if (_stop_flag)
                return QUEUE_STOPPED;
            _task_queue[bucket_index].emplace_back(std::forward<F>(task));
            _cv_not_empty.notify_all();
            return OK;
        }

    public:
        SyncQueue(int _bucket_size, size_t max_size = MaxTaskCount, int wait_timeout = 1) : _bucket_size(_bucket_size), _max_size(max_size), _stop_flag(false), _wait_timeout(wait_timeout)
        {
            size_t bucket_count = (max_size + _bucket_size - 1) / _bucket_size; // 计算需要的桶数量
            _task_queue.resize(bucket_count);                                   // 初始化任务队列的桶
        }
        ~SyncQueue()
        {
            if (!_stop_flag)
                force_stop();
        }
        int put(const T &task, size_t bucket_index)
        {
            return add(task, bucket_index);
        }
        int put(T &&task, size_t bucket_index)
        {
            return add(std::forward<T>(task), bucket_index);
        }
        int take(std::deque<T> &task_list, size_t bucket_index)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            bool tag = _cv_not_empty.wait_for(lock, std::chrono::seconds(_wait_timeout), [=]()
                                              { return !is_empty(bucket_index) || _stop_flag; });
            if (!tag)
                return QUEUE_FULL; // 等待超时，返回队列满的状态
            if (_stop_flag)
                return QUEUE_STOPPED;
            task_list = std::move(_task_queue[bucket_index]);
            _task_queue[bucket_index].clear();
            _cv_not_full.notify_all();
            return OK;
        }
        int take(T &task, size_t bucket_index)
        {
            std::unique_lock<std::mutex> lock(_mutex);
            bool tag = _cv_not_empty.wait_for(lock, std::chrono::seconds(_wait_timeout), [=]() { return !is_empty(bucket_index) || _stop_flag; });
            if (!tag)
                return QUEUE_FULL; // 等待超时，返回队列满的状态
            if (_stop_flag)
                return QUEUE_STOPPED;
            task = std::move(_task_queue[bucket_index].front());
            _task_queue[bucket_index].pop_front();
            _cv_not_full.notify_all();
            return OK;
        }
        void force_stop()
        {
            std::unique_lock<std::mutex> lock(_mutex);
            for (int i = 0; i < _task_queue.size(); ++i)
            {
                while(!_stop_flag && !is_empty(i))
                {
                    _cv_not_full.wait_for(lock, std::chrono::seconds(_wait_timeout));
                }
            }
            _stop_flag = true;
            _cv_not_empty.notify_all();
            _cv_not_full.notify_all();
        }
        bool empty(const size_t &bucket_index) const
        {
            std::unique_lock<std::mutex> lock(_mutex);
            return _task_queue[bucket_index].empty();
        }
        bool full(const size_t &bucket_index) const
        {
            std::unique_lock<std::mutex> lock(_mutex);
            return _task_queue[bucket_index].size() >= _bucket_size;
        }
        size_t size(const size_t &bucket_index) const
        {
            std::unique_lock<std::mutex> lock(_mutex);
            return _task_queue[bucket_index].size();
        }
        size_t count(const size_t &bucket_index) const
        {
            return _task_queue[bucket_index].size();
        }
        void print_task_info() const
        {
            std::unique_lock<std::mutex> lock(_mutex);
            for (size_t i = 0; i < _task_queue.size(); ++i)
            {
                std::cout << "Bucket " << i << ": size = " << _task_queue[i].size() << std::endl;
            }
        }
    };
}
#endif // SYNCQUEUE_HPP
