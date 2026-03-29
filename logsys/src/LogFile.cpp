// Liunx API
#include <sys/types.h>
#include <unistd.h>
// OWNER
#include "LogCommon.hpp"
#include "Timestamp.hpp"
#include "LogFile.hpp"
// C++ STL
#include <string>
#include <sstream>
using namespace std;

namespace shanchuan
{
    // LogFile
    // const std::string basename_;
    // const size_t rollSize_;
    // const int flushInterval_;
    // const int checkEventN_;
    // int count_;
    // time_t startOfPeriod_;
    // time_t lastRoll_;
    // time_t lastFlush_;
    // static const int kRollPerSeconds_ = 60 * 60 * 24;
    //  std::unique_ptr<tulun::AppendFile> file_;
    const std::string hostname()
    {
        char buff[SMALL_BUFF_LEN] = {0};
        if (!::gethostname(buff, SMALL_BUFF_LEN))
        {
            return std::string(buff);
        }
        else
        {
            return std::string("unknownhost");
        }
    }
    pid_t pid()
    {
        return ::getpid();
    }

    void LogFile::append_unlocked(const char *msg, const size_t len)
    {
        file_->append(msg, len);
        if (file_->getWriteBytes() > rollSize_)
        {
            rollFile();
        }
        else
        {
            count_ += 1;
            if (count_ > checkEventN_)
            {
                count_ = 0;
                time_t now = ::time(nullptr);
                time_t thisPeriod = (now / kRollPerSeconds_) * kRollPerSeconds_;
                if (thisPeriod != startOfPeriod_)
                {
                    rollFile();
                }
                else if (now - lastFlush_ > flushInterval_)
                {
                    lastFlush_ = now;
                    file_->flush();
                }
            }
        }
    }
    std::string LogFile::getLogFileName(const std::string &basename, const shanchuan::Timestamp &now)
    {
        std::string filename;
        filename.reserve(basename.size() + SMALL_BUFF_LEN);
        filename = basename;
        filename += ".";
        filename += now.toFormattedFile(); // 20241031090111
        filename += ".";
        filename += hostname();
        std::stringstream ss;
        ss << "." << pid() << ".log";
        filename += ss.str();
        // char buff[40] = {0};
        // sprintf(buff, ".%d.log", pid());
        return filename;
    }
    LogFile::LogFile(const std::string &basename,
                     size_t rollSize,
                     int flushInterval,
                     int checkEventN,
                     bool threadSafe)
        : basename_(basename),
          rollSize_(rollSize),
          flushInterval_(flushInterval),
          checkEventN_(checkEventN),
          mutex_{threadSafe? new std::mutex{}: nullptr},
          count_(0),
          startOfPeriod_(0),
          lastRoll_(0),
          lastFlush_(0)

    {
        rollFile();
    }
    LogFile::~LogFile()
    {
    }
    void LogFile::append(const std::string &msg)
    {
        append(msg.c_str(), msg.size());
    }
    void LogFile::append(const char *msg, const size_t len)
    {
        if(mutex_)
        {
            std::unique_lock<std::mutex> lock(*mutex_);
            append_unlocked(msg, len);
        }
        else
        {
            append_unlocked(msg, len);
        }
    }
    void LogFile::flush()
    {
        file_->flush();
    }
    bool LogFile::rollFile()
    {
        shanchuan::Timestamp now;
        now.now();
        std::string filename = getLogFileName(basename_, now);
        time_t start = (now.getSecond() / kRollPerSeconds_) * kRollPerSeconds_;
        if (now.getSecond() > lastRoll_)
        {
            lastRoll_ = now.getSecond();
            lastFlush_ = now.getSecond();
            startOfPeriod_ = start;
            file_.reset(new shanchuan::AppendFile(filename));
            return true;
        }
        return false;
    }
}