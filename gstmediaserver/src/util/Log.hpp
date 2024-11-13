#ifndef LOG_HPP
#define LOG_HPP


#include <iostream>
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/logger.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>
#include <cxxabi.h>
#include <dlfcn.h>
#include <sstream>
#include "BaseLogger.hpp"

namespace gstmediaserver {

class Logger {
private:
    Logger() {}
    Logger(Logger const&) = delete;
public:

    static std::shared_ptr<spdlog::logger> getLogger() {
        static std::shared_ptr<BaseLogger> mediaServerLogger = nullptr;
        if (!mediaServerLogger) {
            mediaServerLogger = std::make_shared<BaseLogger>("media_server");
        }
        return mediaServerLogger->getLogger();
    }
};

} // namespace gstmediaserver

#endif
