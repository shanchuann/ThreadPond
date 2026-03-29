// C++ STL
#include <string>
#include <memory>
#include <mutex>
using namespace std;
// C API
#include<time.h>

// OWNER
#include"Timestamp.hpp"
#include "AppendFile.hpp"

#ifndef LOGFILE_HPP
#define LOGFILE_HPP

namespace shanchuan
{
    class LogFile
    {
    private:
        const std::string basename_;
        const size_t rollSize_;
        const int flushInterval_;
        const int checkEventN_;
        int count_;

    private:
        time_t startOfPeriod_;
        time_t lastRoll_;
        time_t lastFlush_;
        static const int kRollPerSeconds_ = 60 * 60 * 24;

    private:
        std::unique_ptr<shanchuan::AppendFile> file_;
        std::unique_ptr<std::mutex> mutex_;

    private:
        void append_unlocked(const char *msg, const size_t len);
        static std::string getLogFileName(const std::string &basename,const shanchuan::Timestamp &now);

    public:
        LogFile(const std::string &basename,
                size_t rollSize = 1024 * 128,
                int flushInterval = 3,
                int checkEventN = 30,
                bool threadSafe = true
                );
        ~LogFile();
        void append(const std::string &msg);
        void append(const char *msg, const size_t len);
        void flush();
        bool rollFile();
    };
}
#endif