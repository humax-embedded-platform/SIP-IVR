#include "SipMessageFactory.hpp"
#include "SipMessage.hpp"
#include "SipSdpMessage.hpp"
#include <iostream>
#include "Log.hpp"

std::optional<std::shared_ptr<SipMessage>> SipMessageFactory::createMessage(std::string message)
{
	// std::cout << "Creating SIP message: " << message << std::endl;
	try
	{
		if (containsSdp(message))
		{
            auto msg = std::make_shared<SipSdpMessage>(std::move(message));
			// LOG_D << "msg: " << msg->dump() << ENDL;
			return msg;
		}

        auto msg = std::make_shared<SipSdpMessage>(std::move(message));
		// LOG_D << "msg: " << msg->dump() << ENDL;
		return msg;
	}
	catch (const std::exception&)
	{
		return {};
	}
}

bool SipMessageFactory::containsSdp(const std::string& message) const
{
	return message.find(SDP_CONTENT_TYPE) != std::string::npos;
}
