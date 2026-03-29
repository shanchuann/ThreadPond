#include "Timer.hpp"
#include "TimerQueue.hpp"
#include "Logger.hpp"
#include <functional>
#include <future>
#include <memory>
#include <chrono>

#ifndef SCHEDULED_THREADPOOL_HPP
#define SCHEDULED_THREADPOOL_HPP

namespace shanchuan
{
    class ScheduledThreadPool
    {
    private:
        shanchuan::TimerQueue _task_queue;

    public:
        ScheduledThreadPool()
        {
            LOG_INFO << "Starting ScheduledThreadPool";
        }
        ~ScheduledThreadPool()
        {
            LOG_INFO << "~ScheduledThreadPool() begin, stopping task queue";
            _task_queue.stop();
            LOG_INFO << "~ScheduledThreadPool() end";
        }
        // 在 time 时运行回调函数
        timerId addRunAt(Timestamp time, timerCallBack cb)
        {
            return _task_queue.addTimer(std::move(cb), time, 0);
        }
        // 在延迟 delay 毫秒后运行回调函数
        timerId addRunAfter(size_t delay, timerCallBack cb)
        {
            Timestamp time(shanchuan::addmsTime(Timestamp::Now(), delay));
            return addRunAt(time, std::move(cb));
        }
        // 每隔 interval 毫秒运行一次回调函数
        timerId addRunEvery(size_t interval, timerCallBack cb)
        {
            // debug timestamps
            // fprintf(stderr, "%s\n", shanchuan::Timestamp::Now().toFormattedString().c_str());
            Timestamp time(shanchuan::addmsTime(Timestamp::Now(), interval));
            // fprintf(stderr, "%s\n", time.toFormattedString().c_str());
            return _task_queue.addTimer(std::move(cb), time, interval);
        }
        void Cancel(timerId timerid)
        {
            _task_queue.cancel(timerid);
        }
    };
} // namespace shanchuan
#endif // SCHEDULED_THREADPOOL_HPP