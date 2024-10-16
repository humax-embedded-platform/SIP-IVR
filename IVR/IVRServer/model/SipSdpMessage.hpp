#ifndef SIP_SDP_MESSAGE_HPP
#define SIP_SDP_MESSAGE_HPP

#include "SipMessage.hpp"

class SipSdpMessage : public SipMessage
{
public:
    SipSdpMessage(std::string message);

    void setRtpHost(std::string value);
	void setRtpPort(int port);


    std::string getRtpHost() const;
	int getRtpPort() const;

    std::string toPayload() override;

private:
	void parse() override;
	int extractRtpPort(std::string data) const;
    std::string extractRtpHost(std::string data) const;

    std::string _rtpHost;
	int _rtpPort;
};

#endif
