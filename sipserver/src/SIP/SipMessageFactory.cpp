#include "SipMessageFactory.hpp"
#include <iostream>

namespace sipserver {

std::optional<std::shared_ptr<SipMessage>> SipMessageFactory::createMessage(std::string message, sockaddr_in src)
{
	try
	{
		if (containsSdp(message))
		{
			auto msg = std::make_shared<SipSdpMessage>(std::move(message), std::move(src));
			return msg;
		}

		auto msg = std::make_shared<SipMessage>(std::move(message), std::move(src));
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

} // namespace sipserver

