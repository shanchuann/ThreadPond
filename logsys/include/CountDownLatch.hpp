
#include <mutex>
#include <condition_variable>
using namespace std;
#ifndef COUNT_DOWNLATCH_HPP
#define COUNT_DOWNLATCH_HPP
namespace shanchuan
{
    class CountDownLatch
    {
    private:
        int count_;
        mutable std::mutex mutex_;
        std::condition_variable cond_;
    public:
        CountDownLatch(int count);
        void wait();
        void countDown();
        int getCount() const;
    };
}
#endif