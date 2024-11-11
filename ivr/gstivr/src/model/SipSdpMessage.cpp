#include "SipSdpMessage.hpp"
#include "SipMessageHeaders.h"
#include <string>
#include <cstring>
#include "Log.hpp"

namespace gstivr {

SipSdpMessage::SipSdpMessage(std::string message) : SipMessage(std::move(message))
{
	parse();
	_contentType = "";
}

void SipSdpMessage::setRtpHost(std::string value)
{
    _rtpHost = value;
}

void SipSdpMessage::setRtpPort(int port)
{
	_rtpPort = port;
}

std::string SipSdpMessage::getRtpHost() const
{
    return _rtpHost;
}

int SipSdpMessage::getRtpPort() const
{
	return _rtpPort;
}

std::string SipSdpMessage::mediaDescription() const
{
    return _mediaDes;
}

std::string SipSdpMessage::toPayload()
{
    std::string msg = std::string("v=0") + SipMessageHeaders::HEADERS_DELIMETER +
                 "o=Z 0 20528078 IN IP4 " + _rtpHost + SipMessageHeaders::HEADERS_DELIMETER +
                 "s=Z" + SipMessageHeaders::HEADERS_DELIMETER +
                 "c=IN IP4 " + _rtpHost + SipMessageHeaders::HEADERS_DELIMETER +
                 "t=0 0" + SipMessageHeaders::HEADERS_DELIMETER +
                 "m=audio " + std::to_string(_rtpPort) + " RTP/AVP 106 9 98 101 0 8 3" + SipMessageHeaders::HEADERS_DELIMETER +
                 "a=rtpmap:106 opus/48000/2" + SipMessageHeaders::HEADERS_DELIMETER +
                 "a=fmtp:106 sprop-maxcapturerate=16000; minptime=20; useinbandfec=1" + SipMessageHeaders::HEADERS_DELIMETER +
                 "a=rtpmap:98 telephone-event/48000" + SipMessageHeaders::HEADERS_DELIMETER +
                 "a=fmtp:98 0-16" + SipMessageHeaders::HEADERS_DELIMETER +
                 "a=rtpmap:101 telephone-event/8000" + SipMessageHeaders::HEADERS_DELIMETER +
                 "a=fmtp:101 0-16" + SipMessageHeaders::HEADERS_DELIMETER +
                 "a=sendrecv" + SipMessageHeaders::HEADERS_DELIMETER +
                 "a=rtcp-mux" + SipMessageHeaders::HEADERS_DELIMETER;

    setContentType("application/sdp");
    setContentLength(std::to_string(msg.size()));
    std::string header = SipMessage::toPayload();
    return header + msg;
}

void SipSdpMessage::parse()
{
	std::string msg = _messageStr;

	auto posOfM = msg.find("v=");
	msg.erase(0, posOfM);
	size_t pos = 0;
	while ((pos = msg.find(SipMessageHeaders::HEADERS_DELIMETER)) != std::string::npos)
	{
		std::string line = msg.substr(0, pos);
        if (line.find("m=") != std::string::npos)
		{
			_rtpPort = extractRtpPort(std::move(line));
        } else if (line.find("c=") == 0)
        {
            _rtpHost = extractRtpHost(std::move(line));
            Logger::getLogger()->info("RTP host: {}", _rtpHost);
        } else if (line.find("a=rtpmap:") != std::string::npos)
        {
            if (_mediaDes.empty()) {
                _mediaDes = line.substr(line.find("a=rtpmap:") + 2);
                Logger::getLogger()->info("Media description: {}", _mediaDes);
            }
        }
		msg.erase(0, pos + std::strlen(SipMessageHeaders::HEADERS_DELIMETER));
	}
}

int SipSdpMessage::extractRtpPort(std::string data) const
{
	data.erase(0, data.find(" ") + 1);
	std::string portStr = data.substr(0, data.find(" "));
	return std::stoi(portStr);
}

std::string SipSdpMessage::extractRtpHost(std::string data) const
{
    data.erase(0, data.find("IN IP4 ") + std::strlen("IN IP4 "));
    return data.substr(0, data.find(SipMessageHeaders::HEADERS_DELIMETER));
}
} // namespace gstivr
