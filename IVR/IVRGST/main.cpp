#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "cxxopts.hpp"
#include "Application.hpp"
#include "SipMessage.hpp"
#include "CallDetails.h"
#include "SipSdpMessage.hpp"
#include <iostream>
#include <sstream>
#include <thread>
#include "AppDefines.h"
#include "Log.hpp"

#define SIP_SERVER "192.168.0.4"
#define LOCAL_IP "192.168.0.4"
#define SIP_PORT "5060"
#define CALLER_URI "sip:phong4@192.168.0.4"
#define CALLEE_URI "sip:phong2@192.168.0.4"



int main(int argc, char** argv)
{
    cxxopts::Options options("IVR Application", "Open source application for an automated telephone system");

    options.add_options()
        ("h,help", "Print usage")
        ("i,server-ip", "Sip server ip", cxxopts::value<std::string>())
        ("p,server-port", std::string("Sip server port (default: ") + std::to_string(SEVER_PORT_DEFAULT) + ").", cxxopts::value<int>()->default_value(std::to_string(SEVER_PORT_DEFAULT)))
        ("c,app-ip", "IVR application ip", cxxopts::value<std::string>())
        ("m,app-port", std::string("IVR application port (default: ") + std::to_string(MEDIA_SERVER_PORT_DEFAULT) + ").", cxxopts::value<int>()->default_value(std::to_string(MEDIA_SERVER_PORT_DEFAULT)));

    auto result = options.parse(argc, argv);

    if (result.count("help"))
    {
        Logger::getLogger()->info(options.help());
        exit(0);
    }

    try
    {
        std::string server_ip = result["server-ip"].as<std::string>();
        int server_port = result["server-port"].as<int>();
        std::string app_ip = result["app-ip"].as<std::string>();
        int app_port = result["app-port"].as<int>();

        Application app(server_ip, server_port, app_ip, app_port);
        Logger::getLogger()->info("Application has been started ...");
        getchar();
    }
    catch (const cxxopts::OptionException&)
    {
        Logger::getLogger()->info("Please enter ip and port.");
    }
}
