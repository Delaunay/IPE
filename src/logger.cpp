#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "logger.h"

#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <unordered_map>
#include <memory>

// Linux signal handling & stack trace printing
// --------------------------------------------
#ifdef __linux__
#include <execinfo.h>
#include <signal.h>
#endif

namespace ipe {
#ifdef __linux__
std::vector<std::string> get_backtrace(size_t size){
    std::vector<void *> ptrs(size);

    int real_size = backtrace(ptrs.data(), int(ptrs.size()));

    char ** symbols = backtrace_symbols(ptrs.data(), int(size));

    std::vector<std::string> names((size_t(real_size)));

    for (int i = 0; i < real_size; ++i){
        const char* line = symbols[i];
        names[size_t(i)] = std::string(line);
    }

    return names;
}

void show_backtrace() {
    std::vector<std::string> symbols = get_backtrace(16);
    for (auto& sym: symbols){
        spdlog_log(LogLevel::CRITICAL, sym);
    }
}

void signal_handler(int sig){
    spdlog_log(LogLevel::CRITICAL, fmt::format("Received signal {} >>>", sig));
    show_backtrace();
    spdlog_log(LogLevel::CRITICAL, "<<< Exiting");
    exit(1);
}

int register_signal_handler(){
    signal(SIGSEGV, signal_handler);   // install our handler
    return 0;
}
#else
void register_signal_handler(){}

void show_backtrace(){}

std::vector<std::string> get_backtrace(size_t size){
    return std::vector<std::string>();
}
#endif
// ==============================================================


// Logging
using Logger = std::shared_ptr<spdlog::logger>;

Logger new_logger(char const* name){
    // Static so only executed once
    static int _ = register_signal_handler();

    spdlog::enable_backtrace(32);

    auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    auto console = std::make_shared<spdlog::logger>(name, stdout_sink);

    console->set_level(spdlog::level::level_enum::trace);
    console->flush_on(spdlog::level::level_enum::trace);

    spdlog::register_logger(console);
    // Logger Type  : for filtering
    // date         : know which line came first
    // thread id    : in case of multithreaded work
    // message
    spdlog::set_pattern("[%L] [%d-%m-%Y %H:%M:%S.%e] [%t] %v");

    return console;
}

Logger root(){
    static Logger log = new_logger("root");
    return log;
}

static constexpr spdlog::level::level_enum log_level_spd[] = {
    spdlog::level::level_enum::trace,
    spdlog::level::level_enum::debug,
    spdlog::level::level_enum::info,
    spdlog::level::level_enum::warn,
    spdlog::level::level_enum::err,
    spdlog::level::level_enum::critical,
    spdlog::level::level_enum::off,
};

void show_log_backtrace(){
    spdlog::dump_backtrace();
}

void spdlog_log(LogLevel level, std::string const& msg){
    root()->log(log_level_spd[int(level)], msg);
}

const char* Exception::what() const noexcept {
    show_backtrace();
    return message;
}
}
