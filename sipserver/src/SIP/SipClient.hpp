#ifndef SIP_CLIENT_HPP
#define SIP_CLIENT_HPP

#include <netinet/in.h>
#include <iostream>

namespace sipserver {

class SipClient
{
public:
	SipClient(std::string number, sockaddr_in address);

	bool operator==(SipClient other);

	std::string getNumber() const;
	sockaddr_in getAddress() const;
    void setAddress(sockaddr_in address);
    std::string getIp() const;
    uint16_t getPort() const;
    uint16_t getRtpPort() const;
    void setRtpPort(uint16_t rtpPort);
    std::string getTag() const;
    void setTag(std::string tag);
    std::string mediaDescContent() const;
    void setMediaDescContent(std::string mediaDescContent);

private:
	std::string _number;
	sockaddr_in _address;
    uint16_t _rtpPort;
    std::string _tag;
    std::string _mediaDescContent;
};

} // namespace sipserver

#endif
