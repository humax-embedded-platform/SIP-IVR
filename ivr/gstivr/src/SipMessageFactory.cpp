#include "SipMessageFactory.hpp"
#include "SipMessage.hpp"
#include "SipSdpMessage.hpp"
#include <iostream>
#include "Log.hpp"

namespace gstivr {

std::optional<std::shared_ptr<SipMessage>> SipMessageFactory::createMessage(std::string message)
{
	try
	{
		if (containsSdp(message))
		{
            auto msg = std::make_shared<SipSdpMessage>(std::move(message));
			return msg;
		}

        auto msg = std::make_shared<SipMessage>(std::move(message));
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
} // namespace gstivr

