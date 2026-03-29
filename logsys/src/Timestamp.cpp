// time ;ctime
// sprintf; stringostream
// liunx api
#include <sys/time.h>
// C++ STL
#include <string>
#include <sstream>
using namespace std;
// C++ api
#include <ctime>
#include <stdio.h>
#include "Timestamp.hpp"

namespace shanchuan
{
    static const int slen = 128;
    // class Timestamp
    // std::uint64_t micr_;

    Timestamp::Timestamp() : micr_(0) {}
    Timestamp::Timestamp(uint64_t m) : micr_(m) {}
    Timestamp::~Timestamp() {}

    void Timestamp::swap(Timestamp &other)
    {
        std::swap(this->micr_, other.micr_);
    }
    std::string Timestamp::toString() const
    {
        char buff[slen] = {};
        time_t sec = micr_ / kMinPerSec;
        time_t mic = micr_ % kMinPerSec;
        sprintf(buff, "%lu.%lu", sec, mic);
        return std::string(buff);
    }
    // 123456543.3454;
    std::string Timestamp::toFormattedString(bool showmicro) const
    {
        char buff[slen] = {};
        time_t sec = micr_ / kMinPerSec;
        time_t mic = micr_ % kMinPerSec;
        struct tm dtm = {};
        //gmtime_r(&sec, &dtm);
        localtime_r(&sec, &dtm);
        int pos = sprintf(buff, "%04d/%02d/%02d-%02d:%02d:%02d",
                          dtm.tm_year + 1900,
                          dtm.tm_mon + 1,
                          dtm.tm_mday,
                          dtm.tm_hour,
                          dtm.tm_min,
                          dtm.tm_sec);
        if (showmicro)
        {
            sprintf(buff + pos, ".%ldZ", mic);
        }
        return std::string(buff);
    }
    // 2024/07/01 08:20:20.34567Z
    std::string Timestamp::toFormattedFile() const
    {
        char buff[slen] = {};
        time_t sec = micr_ / kMinPerSec;
        time_t mic = micr_ % kMinPerSec;
        struct tm dtm = {};
        // gmtime_r(&sec, &dtm);
        localtime_r(&sec, &dtm);
        int pos = sprintf(buff, "%04d%02d%02d%02d%02d%02d",
                          dtm.tm_year + 1900,
                          dtm.tm_mon + 1,
                          dtm.tm_mday,
                          dtm.tm_hour,
                          dtm.tm_min,
                          dtm.tm_sec);
        sprintf(buff + pos, ".%ldZ", mic);
        return std::string(buff);
    }
    // 20240701082020
    bool Timestamp::valid() const { return micr_ > 0; }
    
    uint64_t Timestamp::getMicro() const
    {
        return micr_;
    }
    uint64_t Timestamp::getMilli() const
    {
        return (micr_ / KMinPerMil);
    }
    uint64_t Timestamp::getSecond() const
    {
        return (micr_ / kMinPerSec);
    }
    const Timestamp &Timestamp::now()
    {
        *this = Now();
        return *this;
    }
    Timestamp::operator uint64_t() const
    {
        return micr_;
    }

    Timestamp Timestamp::Now()
    {
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        return Timestamp(tv.tv_sec * kMinPerSec + tv.tv_usec);
    }
    Timestamp Timestamp::Invalid()
    {
        return Timestamp(0);
    }
}