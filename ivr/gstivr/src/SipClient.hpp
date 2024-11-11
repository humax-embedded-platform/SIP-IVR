#ifndef SIP_CLIENT_HPP
#define SIP_CLIENT_HPP


#include <netinet/in.h>
#include <iostream>

namespace gstivr {

class SipClient
{
public:
	SipClient(std::string number, sockaddr_in address);

	bool operator==(SipClient other);

	std::string getNumber() const;
	sockaddr_in getAddress() const;
    std::string getIp() const;
    uint16_t getPort() const;
private:
	std::string _number;
	sockaddr_in _address;
};

} // namespace gstivr

#endif
