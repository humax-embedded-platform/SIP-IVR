#include <iostream>
#include "sipserver.h"
#include "gstivr.h"
#include "gstmediaserver.h"
#include "cxxopts.hpp"
#include "BaseLogger.hpp"

#define SEVER_PORT_DEFAULT 5060

int main(int argc, char *argv[]) {

    BaseLogger::initLogger();

    BaseLogger logger("main");

    cxxopts::Options options("IVR System", "Open source application for an automated telephone system");

    options.add_options()
        ("h,help", "Print usage")
        ("i,server-ip", "Sip server ip", cxxopts::value<std::string>())
        ("p,server-port", std::string("Sip server port (default: ") + std::to_string(SEVER_PORT_DEFAULT) + ").", cxxopts::value<int>()->default_value(std::to_string(SEVER_PORT_DEFAULT)))
#if 0
        ("c,app-ip", "IVR application ip", cxxopts::value<std::string>())
#endif
        ("m,media-blob", "Media blob directory path", cxxopts::value<std::string>());

    auto result = options.parse(argc, argv);

    if (result.count("help"))
    {
        logger.getLogger()->info(options.help());
        exit(0);
    }

    try
    {
        std::string server_ip = result["server-ip"].as<std::string>();
        int server_port = result["server-port"].as<int>();
#if 0
        std::string app_ip = result["app-ip"].as<std::string>();
#endif
        std::string media_blob = result["media-blob"].as<std::string>();

        // set media_blob to env
        setenv("MEDIA_DIR", media_blob.c_str(), 1);

        // start sip server
        sipserver::start(server_ip, server_port);

        // start MediaServer at new thread
        gstmediaserver::start();

        // start ivr application
        gstivr::start(server_ip, server_port, server_ip);

        getchar();
    }
    catch (const cxxopts::OptionException&)
    {
        logger.getLogger()->info("Input parameters are not correct. Please check the help message.");
    }

    return 0;
}
