
// OWNER
#include "LogMessage.hpp"
// C++ STL
#include <string>
#include <functional>

#ifndef LOGGER_HPP
#define LOGGER_HPP

namespace shanchuan
{
    class Logger
    {
    public:
        using OutputFun = std::function<void(const std::string &)>;
        using FlushFun = std::function<void(void)>;

    public:
        static OutputFun s_output_;
        static FlushFun s_flush_;
        static void SetOuput(OutputFun fun);
        static void SetFlush(FlushFun fun);

    private:
        shanchuan::LogMessage impl_;

    public:
        Logger(const shanchuan::LOG_LEVEL &level,
               const string &filename,
               const string &funcname,
               const int line);
        ~Logger();
        shanchuan::LogMessage &stream();

    private:
        static shanchuan::LOG_LEVEL s_level_;

    public:
        static shanchuan::LOG_LEVEL getLogLevel();
        static void SetLogLevel(const LOG_LEVEL &level);
    };

    //-------------------------------------------

#define LOG_TRACE                                                \
    if (shanchuan::Logger::getLogLevel() <= shanchuan::LOG_LEVEL::TRACE) \
    shanchuan::Logger(shanchuan::LOG_LEVEL::TRACE, __FILE__, __func__, __LINE__).stream()

#define LOG_DEBUG                                                 \
if (shanchuan::Logger::getLogLevel() <= shanchuan::LOG_LEVEL::DEBUG) \
    shanchuan::Logger(shanchuan::LOG_LEVEL::DEBUG, __FILE__, __func__, __LINE__).stream()

#define LOG_INFO                                                 \
    if (shanchuan::Logger::getLogLevel() <= shanchuan::LOG_LEVEL::INFO) \
    shanchuan::Logger(shanchuan::LOG_LEVEL::INFO, __FILE__, __func__, __LINE__).stream()

#define LOG_WARN \
    shanchuan::Logger(shanchuan::LOG_LEVEL::WARN, __FILE__, __func__, __LINE__).stream()

#define LOG_ERROR \
    shanchuan::Logger(shanchuan::LOG_LEVEL::ERROR, __FILE__, __func__, __LINE__).stream()

#define LOG_FATAL \
    shanchuan::Logger(shanchuan::LOG_LEVEL::FATAL, __FILE__, __func__, __LINE__).stream()

#define LOG_SYSERR \
    shanchuan::Logger(shanchuan::LOG_LEVEL::ERROR, __FILE__, __func__, __LINE__).stream()

#define LOG_SYSFATAL \
    shanchuan::Logger(shanchuan::LOG_LEVEL::FATAL, __FILE__, __func__, __LINE__).stream()
}

#endif