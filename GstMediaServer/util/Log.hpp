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

class Logger {
private:
    Logger() {}
    Logger(Logger const&) = delete;
public:
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
        Logger::getLogger()->error("Received signal: {}", sig);
        Logger::getLogger()->error("Backtrace:\n{}", trace_buf.str());
        free(symbols);
        exit(1);
    }

    static std::shared_ptr<spdlog::logger> getLogger() {
        static std::shared_ptr<spdlog::logger> logger = nullptr;
        if (!logger) {
#ifdef DEBUG_MODE
            spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %^%v%$");
            spdlog::set_level(spdlog::level::trace);
            logger = spdlog::stdout_color_mt("console");
#else
            auto max_size = 1048576 * 5;
            auto max_files = 3;
            logger = spdlog::rotating_logger_mt("ms_logger", "logs/media_server_rotating.txt", max_size, max_files);
#endif
            spdlog::enable_backtrace(32);

            struct sigaction sa;
            sa.sa_handler = segfaultHandler;
            sigemptyset(&sa.sa_mask);
            sa.sa_flags = SA_RESTART;
            sigaction(SIGSEGV, &sa, NULL);
        }
        return logger;
    }
};

#endif
