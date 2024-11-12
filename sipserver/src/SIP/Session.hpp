#ifndef SESSION_HPP
#define SESSION_HPP

#include <memory>
#include "SipClient.hpp"

namespace sipserver {

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


    Session(std::string callID, std::shared_ptr<SipClient> src, uint32_t rtpPort);

	void setState(State state);
    void setDest(std::shared_ptr<SipClient> dest, uint32_t rtpPort);
    void setReferedDest(std::shared_ptr<SipClient> dest, uint32_t rtpPort);

	std::string getCallID() const;
	std::shared_ptr<SipClient> getSrc() const;
	std::shared_ptr<SipClient> getDest() const;
    std::shared_ptr<SipClient> getReferedDest() const;
	State getState() const;

    std::string curOriginTransaction() const;
    void setCurOriginTransaction(const std::string& transaction);

    std::string curReferedTransaction() const;
    void setCurReferedTransaction(const std::string& transaction);

    uint32_t getSrcRtpPort() const;
    uint32_t getDestRtpPort() const;
    uint32_t getReferedDestRtpPort() const;

    std::string getFromTag() const;
    void setFromTag(std::string fromTag);
    std::string getToTag() const;
    void setToTag(std::string toTag);
    std::string getReferedToTag() const;
    void setReferedToTag(std::string toTag);

    std::string fromMediaDescContent();
    void setFromMediaDescContent(std::string mediaDescContent);
    std::string toMediaDescContent();
    void setToMediaDescContent(std::string mediaDescContent);
    std::string referedToMediaDescContent();
    void setReferedToMediaDescContent(std::string mediaDescContent);

private:
    State _state;
	std::string _callID;
	std::shared_ptr<SipClient> _src;
	std::shared_ptr<SipClient> _dest;
    std::shared_ptr<SipClient> _referedDest;
    std::string _curOriginTransaction;
    std::string _curReferedTransaction;

    uint32_t _srcRtpPort;
    uint32_t _destRtpPort;
    uint32_t _referedDestRtpPort;

    std::string _fromTag;
    std::string _toTag;
    std::string _referedToTag;

    std::string _fromMediaDescContent;
    std::string _toMediaDescContent;
    std::string _referedToMediaDescContent;
};

} // namespace sipserver

#endif
