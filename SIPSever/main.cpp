#include <iostream>
#include "SipServer.hpp"
#include "cxxopts.hpp"
#include "Log.hpp"

int main(int argc, char** argv)
{
	cxxopts::Options options("SipServer", "Open source server for handling voip calls based on sip.");

	options.add_options()
		("h,help", "Print usage")
		("i,ip", "Sip server ip", cxxopts::value<std::string>())
		("p,port", "Sip server ip.", cxxopts::value<int>()->default_value(std::to_string(5060)));

	auto result = options.parse(argc, argv);

	if (result.count("help"))
	{
        Logger::getLogger()->info(options.help());
		exit(0);
	}

	try
	{
		std::string ip = result["ip"].as<std::string>();
		int port = result["port"].as<int>();
        SipServer server(std::move(ip), port);
        Logger::getLogger()->info("Server has been started. Listening...");
		getchar();
	}
	catch (const cxxopts::OptionException&)
    {
        Logger::getLogger()->error("Please enter ip and port.");
	}
	return 0;
}
