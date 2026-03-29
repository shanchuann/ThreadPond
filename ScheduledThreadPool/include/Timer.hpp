#include <functional>
#include <utility>
#include "Timestamp.hpp"
#include "Notcopy.hpp"

#ifndef TIMER_HPP
#define TIMER_HPP

namespace shanchuan
{
    using timerCallBack = std::function<void(void)>;
    class Timer : public Notcopy
    {
    private:
        int _timerfd;
        timerCallBack _callback;
        shanchuan::Timestamp _expiration; // 过期时间点
        size_t _interval;                 // 时间间隔，单位为毫秒
        bool _is_repeating;               // 是否重复定时
    public:
        Timer();
        ~Timer();
        Timer(Timer &&other) noexcept;
        Timer &operator=(Timer &&other) noexcept;
        bool init(timerCallBack cb, const shanchuan::Timestamp &when, size_t interval);
        bool resetTimer(Timestamp newtime);
        bool setTimer();
        void handleEvent();
        int getTimerId() const;
        bool isRepeating() const;
        bool closeTimer();
    };
    using timerId = std::pair<int, Timer *>;
} // namespace shanchuan
#endif // TIMER_HPP