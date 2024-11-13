#ifndef BASELOGGER_H
#define BASELOGGER_H

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
#include <fstream>
#include <memory>
#include <map>
#include <chrono>
#include <iomanip>

static std::map<std::string, std::shared_ptr<spdlog::logger>> _loggers;

class BaseLogger {
private:
    BaseLogger() {}
    BaseLogger(BaseLogger const&) = delete;
public:
    BaseLogger(std::string context) {
        _contextName = context;
    }
    static void initLogger() {
        struct sigaction sa;
        sa.sa_handler = segfaultHandler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;
        sigaction(SIGSEGV, &sa, NULL);
    }

    std::shared_ptr<spdlog::logger> getLogger() {
        std::shared_ptr<spdlog::logger> logger = nullptr;
        if (_loggers.find(_contextName) != _loggers.end()) {
            logger = _loggers[_contextName];
            return logger;
        }


        if (!logger) {
#ifdef DEBUG_MODE
            spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %^%v%$");
            spdlog::set_level(spdlog::level::trace);
            logger = spdlog::stdout_color_mt(_contextName + "_logger");
#else
            auto max_size = 1048576 * 5;
            auto max_files = 3;
            logger = spdlog::rotating_logger_mt(_contextName + "_logger", std::string("logs/") + _contextName + "_rotating.txt", max_size, max_files);
#endif
            _loggers[_contextName] = logger;
        }
        return logger;
    }

private:
    static void segfaultHandler(int sig) {
        (void)sig;
        void *callstack[128];
        const int nMaxFrames = sizeof(callstack) / sizeof(callstack[0]);
        char buf[1024];
        int nFrames = backtrace(callstack, nMaxFrames);
        char **symbols = backtrace_symbols(callstack, nFrames);

        std::ostringstream trace_buf;
        for (int i = 0; i < nFrames; i++) {
            Dl_info info;
            if (dladdr(callstack[i], &info)) {
                char *demangled = NULL;
                int status;
                demangled = abi::__cxa_demangle(info.dli_sname, NULL, 0, &status);
                std::snprintf(
                    buf,
                    sizeof(buf),
                    "#%-3d %*p %s + %zd\n",
                    i,
                    (int)(2 + sizeof(void*) * 2),
                    callstack[i],
                    status == 0 ? demangled : info.dli_sname,
                    (char *)callstack[i] - (char *)info.dli_saddr);
                free(demangled);
            } else {
                std::snprintf(buf, sizeof(buf), "#%-3d %*p\n",
                              i, (int)(2 + sizeof(void*) * 2), callstack[i]);
            }
            trace_buf << buf;
        }

        // save to crashed log file, filename by timestamp
        auto now = std::chrono::system_clock::now();
        std::time_t timeNow = std::chrono::system_clock::to_time_t(now);
        std::tm* localTime = std::localtime(&timeNow);
        std::ostringstream oss;
        oss << std::put_time(localTime, "%Y-%m-%d_%H-%M-%S");
        std::string filename = std::string("logs/crashed_log_") + oss.str() + ".txt";
        std::ofstream file(filename);
        file << "Received signal: " << sig << std::endl;
        file << "Backtrace:\n" << trace_buf.str() << std::endl;
        file.close();

        std::cerr  << "Received signal: " << sig << std::endl;
        std::cerr  << "Backtrace:\n" << trace_buf.str() << std::endl;

        free(symbols);
        exit(1);
    }

private:
    std::string _contextName;
};


#endif
