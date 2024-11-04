#include "SipClient.hpp"
#include <arpa/inet.h>
#include <netinet/in.h>

SipClient::SipClient(std::string number, sockaddr_in address) : _number(std::move(number)), _address(std::move(address))
{
}

bool SipClient::operator==(SipClient other)
{
	if (_number == other.getNumber()) 
	{
		return true;
	}

	return false;
}

std::string SipClient::getNumber() const
{
	return _number;
}

sockaddr_in SipClient::getAddress() const
{
	return _address;
}

std::string SipClient::getIp() const
{
    return std::string(inet_ntoa(_address.sin_addr));
}

uint16_t SipClient::getPort() const
{
    return ntohs(_address.sin_port);
}

uint16_t SipClient::getRtpPort() const
{
    return _rtpPort;
}

void SipClient::setRtpPort(uint16_t rtpPort)
{
    _rtpPort = rtpPort;
}

std::string SipClient::getTag() const
{
    return _tag;
}

void SipClient::setTag(std::string tag)
{
    _tag = std::move(tag);
}

std::string SipClient::mediaDescContent() const
{
    return _mediaDescContent;
}

void SipClient::setMediaDescContent(std::string mediaDescContent)
{
    _mediaDescContent = std::move(mediaDescContent);
}
