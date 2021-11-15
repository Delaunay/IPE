#ifndef PROJECT_TEST_SRC_LOGGER_HEADER
#define PROJECT_TEST_SRC_LOGGER_HEADER

#include <string>
#include <vector>
// Do not include spdlog directly
// only use the fmt header
#include <spdlog/fmt/bundled/core.h>

#include "version.h"

namespace ipe
{

// Path to repository on current system
constexpr char __source_dir[] = _SOURCE_DIRECTORY;
// Length of the path so we can cut unimportant folders
constexpr int __size_src_dir = sizeof(_SOURCE_DIRECTORY) / sizeof(char);

struct CodeLocation{
    CodeLocation(std::string const& file, std::string const& fun, int line, std::string const& fun_long):
        filename(file.substr(__size_src_dir)), function_name(fun), line(line), function_long(fun_long)
    {}

    std::string filename;
    std::string function_name;
    int line;
    std::string function_long;
};

#define LOC ipe::CodeLocation(__FILE__, __FUNCTION__, __LINE__, __PRETTY_FUNCTION__)

enum class LogLevel{
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    CRITICAL,
    OFF
};

// Show backtrace using spdlog
void show_log_backtrace();

// Show backtrace using execinfo
void show_backtrace();

// retrieve backtrace using execinfo
std::vector<std::string> get_backtrace(size_t size);

void spdlog_log(LogLevel level, const std::string& msg);

template<typename ... Args>
void log(LogLevel level, CodeLocation const& loc, const char* fmt, const Args& ... args){
    auto msg = fmt::format("{}:{} {} - {}",
                           loc.filename,
                           loc.line,
                           loc.function_name,
                           fmt::format(fmt, args...));

    spdlog_log(level, msg);
}

#define CXX_HELPER_LOG_HELPER(level, ...) log(level, LOC, __VA_ARGS__)

#define info(...)       CXX_HELPER_LOG_HELPER(ipe::LogLevel::INFO, __VA_ARGS__)
#define warn(...)       CXX_HELPER_LOG_HELPER(ipe::LogLevel::WARN, __VA_ARGS__)
#define debug(...)      CXX_HELPER_LOG_HELPER(ipe::LogLevel::DEBUG, __VA_ARGS__)
#define error(...)      CXX_HELPER_LOG_HELPER(ipe::LogLevel::ERROR, __VA_ARGS__)
#define critical(...)   CXX_HELPER_LOG_HELPER(ipe::LogLevel::CRITICAL, __VA_ARGS__)


// Exception that shows the backtrace when .what() is called
class Exception: public std::exception{
public:
    template<typename ... Args>
    Exception(const char* fmt, const Args& ... args):
        message(fmt::format(fmt, args...).c_str())
    {}

    const char* what() const noexcept final;

private:
    const char* message;
};

// Make a simple exception
#define NEW_EXCEPTION(name)\
    class name: public ipe::Exception{\
    public:\
        template<typename ... Args>\
        name(const char* fmt, const Args& ... args):\
            Exception(fmt, args...)\
        {}\
    };
}

#endif
