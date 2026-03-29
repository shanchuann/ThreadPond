
// C++ STL
#include<atomic>
#include<string>
#include<memory>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<vector>
using namespace std;
// owner
#include "LogFile.hpp"
#include "CountDownLatch.hpp"
#ifndef ASYN_LOGGING_HPP
#define ASYN_LOGGING_HPP

namespace shanchuan
{
    class AsynLogging
    {
    private:
        void workthreadfunc();
    private:
        const int flushInterval_; // 3s;
        std::atomic<bool> running_;
        const std::string basename_;
        const size_t rollSize_;
        std::unique_ptr< std::thread > pthread_;
        std::mutex mutex_;
        std::condition_variable cond_;
        std::string currentBuffer_;
        std::vector<std::string> buffers_;
        shanchuan::LogFile output_;
        shanchuan::CountDownLatch latch_;

    public:
        AsynLogging(const std::string &basename,
                    const size_t rollSize = 1024*128,
                    int flushInterval = 3);
        ~AsynLogging();
        void append(const std::string &msg);
        void append(const char *msg, const size_t len);
        void start();
        void stop();
        void flush();
    };
}
#endif