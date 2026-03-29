
// C API
#include <stdio.h>
// C++ stl
#include <string>
#include<memory>
using namespace std;
#ifndef APPENDFILE_HPP
#define APPENDFILE_HPP

namespace shanchuan
{
    class AppendFile
    {
    private:
        static const size_t FILE_BUFF_SIZE = 128 * 1024; // 128k;
        std::unique_ptr<char[]> buffer_; 
        FILE *fp_;
        size_t writenBytes_;
        size_t write(const char *msg, const size_t len);
    public:
        AppendFile(const std::string &filename);
        ~AppendFile();
        void append(const std::string &msg);
        void append(const char *msg, const size_t len);
        void flush();
        size_t getWriteBytes() const;
    };
}

#endif