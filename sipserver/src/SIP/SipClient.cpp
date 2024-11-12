#include "SipClient.hpp"
#include <arpa/inet.h>
#include <netinet/in.h>

namespace sipserver {

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

void SipClient::setAddress(sockaddr_in address)
{
    _address = std::move(address);
}
} // namespace sipserver
