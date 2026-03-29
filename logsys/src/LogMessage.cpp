//owner
#include"LogMessage.hpp"
#include"Timestamp.hpp"
namespace shanchuan
{
    // class LogMessage
    // std::string header_;
    // std::string text_;
    // tulun::LOG_LEVEL level_;

    LogMessage::LogMessage(const shanchuan::LOG_LEVEL &level,
                           const std::string &filename,
                           const std::string &funcname,
                           const int line)
        :header_{},text_{},level_(level)
    {
        std::stringstream ss;
        ss << shanchuan::Timestamp::Now().toFormattedString()<<" ";
        ss << shanchuan::LLTOSTR[static_cast<int>(level_)] << " ";
        const size_t pos = filename.find_last_of('/');
        const std::string fname = filename.substr(pos + 1);
        ss << fname << " " << funcname << " " << line << " ";
        header_ = ss.str();
    }
    LogMessage ::~LogMessage()
    {
    }
    const shanchuan::LOG_LEVEL & LogMessage::getLogLevel() const
    {
        return level_;
    }
    const std::string LogMessage::toString() const
    {
        std::ostringstream oss;
        oss << header_;
        oss << ": ";
        oss << text_;
        std::string out = oss.str();
        if (out.empty() || out.back() != '\n')
        {
            out.push_back('\n');
        }
        return out;
    }
}