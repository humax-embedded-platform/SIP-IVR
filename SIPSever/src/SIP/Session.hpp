#ifndef SESSION_HPP
#define SESSION_HPP

#include <memory>

#include "SipClient.hpp"

class Session
{
public:

	enum class State
	{
		Invited,
		Busy,
		Unavailable,
		Cancel,
		Bye,
		Connected,
	};


	Session(std::string callID, std::shared_ptr<SipClient> src, uint32_t srcRtpPort);

	void setState(State state);
    void setDest(std::shared_ptr<SipClient> dest, uint32_t rtpPort);
    void setReferedDest(std::shared_ptr<SipClient> dest, uint32_t rtpPort);

	std::string getCallID() const;
	std::shared_ptr<SipClient> getSrc() const;
	std::shared_ptr<SipClient> getDest() const;
    std::shared_ptr<SipClient> getReferedDest() const;
	uint32_t getSrcRtpPort() const;
	uint32_t getDestRtpPort() const;
    uint32_t getReferedDestRtpPort() const;
	State getState() const;

private:
	std::string _callID;
	std::shared_ptr<SipClient> _src;
	std::shared_ptr<SipClient> _dest;
    std::shared_ptr<SipClient> _referedDest;

	State _state;

	uint32_t _srcRtpPort;
	uint32_t _destRtpPort;
    uint32_t _referedDestRtpPort;
};

#endif
