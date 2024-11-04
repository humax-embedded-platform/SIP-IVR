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


    Session(std::string callID, std::shared_ptr<SipClient> src);

	void setState(State state);
    void setDest(std::shared_ptr<SipClient> dest);
    void setReferedDest(std::shared_ptr<SipClient> dest);

	std::string getCallID() const;
	std::shared_ptr<SipClient> getSrc() const;
	std::shared_ptr<SipClient> getDest() const;
    std::shared_ptr<SipClient> getReferedDest() const;
	State getState() const;

private:
    State _state;
	std::string _callID;
	std::shared_ptr<SipClient> _src;
	std::shared_ptr<SipClient> _dest;
    std::shared_ptr<SipClient> _referedDest;

};

#endif
