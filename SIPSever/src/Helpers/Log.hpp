#ifndef LOG_HPP
#define LOG_HPP

#include <iostream>
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/logger.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

class Logger {
private:
    Logger() {}
    Logger(Logger const&) = delete;
public:
    static std::shared_ptr<spdlog::logger> getLogger() {
        static std::shared_ptr<spdlog::logger> logger = nullptr;
        if (!logger) {
#ifdef DEBUG_MODE
            spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
            spdlog::set_level(spdlog::level::debug); // Set global log level to debug
            logger = spdlog::stdout_color_mt("console");
#else
            auto max_size = 1048576 * 5;
            auto max_files = 3;
            logger = spdlog::rotating_logger_mt("some_logger_name", "logs/sip_server_rotating.txt", max_size, max_files);
#endif
        }
        return logger;
    }
};

#endif
