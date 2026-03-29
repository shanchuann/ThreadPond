
#include "CountDownLatch.hpp"

namespace shanchuan
{
    CountDownLatch::CountDownLatch(int count)
        : count_(count)
    {
    }
    void CountDownLatch::wait()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (count_ > 0)
        {
            cond_.wait(lock);
        }
    }
    void CountDownLatch::countDown()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        count_ -= 1;
        if (count_ == 0)
        {
            cond_.notify_all();
        }
    }
    int CountDownLatch::getCount() const
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return count_;
    }
}