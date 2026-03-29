

// C++ STL;
#include <string>
//using namespace std;
// C API
#include<stdint.h>

#ifndef TIMESTAMP_HPP
#define TIMESTAMP_HPP

namespace shanchuan
{
    class Timestamp
    {
    private:
        uint64_t micr_; // 微秒数
    public:
        Timestamp();
        Timestamp(uint64_t m);
        ~Timestamp();
        void swap(Timestamp &other);
        std::string toString() const;
        // 123456543.3454;
        std::string toFormattedString(bool showmicro = true) const;
        // 2024/07/01 08:20:20.34567Z
        std::string toFormattedFile() const;
        // 20240701082020
        bool valid() const;
        uint64_t getMilli() const;
        uint64_t getMicro() const; 
        uint64_t getSecond() const;
        const Timestamp &now();
        operator uint64_t() const;
    public:
        static Timestamp Now();
        static Timestamp Invalid();
        static const int kMinPerSec = 1000 * 1000;
        static const int KMinPerMil = 1000;
    };

    //
    inline time_t diffSecond(const Timestamp &a,const Timestamp &b)
    {
        return a.getSecond() - b.getSecond();
    }

    inline time_t diffMilli(const Timestamp &a,const Timestamp &b)
    {
        return a.getMilli() - b.getMilli();
    }
    inline time_t diffMicro(const Timestamp &a, const Timestamp &b)
    {
        return a.getMicro() - b.getMicro();
    }
    

    // add seconds;
    inline Timestamp addsTime(const Timestamp &timestamp,double seconds)
    {
        int64_t delta = static_cast<int64_t>(Timestamp::kMinPerSec * seconds);
        return Timestamp(timestamp.getMicro() + delta);
    }
    
    // add millisecods;
    inline Timestamp addmsTime(const Timestamp &timestamp,double mil)
    {
        return Timestamp(timestamp.getMicro() + mil * 1000);
    }
    // add micro;
    inline Timestamp addmcTime(const Timestamp &timestamp,double mis)
    {
        return Timestamp(timestamp.getMicro() + mis);
    }

}
#endif 