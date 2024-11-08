#ifndef SIP_CLIENT_HPP
#define SIP_CLIENT_HPP

#ifdef __linux__
#include <netinet/in.h>
#elif defined _WIN32 || defined _WIN64
#include <WinSock2.h>
#endif

#include <iostream>

class SipClient
{
public:
	SipClient(std::string number, sockaddr_in address);

	bool operator==(SipClient other);

	std::string getNumber() const;
	sockaddr_in getAddress() const;
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

#endif
