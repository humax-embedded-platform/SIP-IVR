#include "CallSession.hpp"
#include "Log.hpp"

CallSession::CallSession(std::string callID) :
    _callID(std::move(callID)), _state(State::Invited)
{
}

void CallSession::setState(State state)
{
	if (state == _state)
		return;
	_state = state;
	if (state == State::Connected)
	{
        LOG_D << "Session Created " << _callID << ENDL;
	}
}

void CallSession::setDest(std::shared_ptr<SipClient> dest, uint32_t destRtpPort)
{
	_dest = dest;
	_destRtpPort = destRtpPort;
}

void CallSession::setSrc(std::shared_ptr<SipClient> src, uint32_t srcRtpPort)
{
    _src = src;
    _srcRtpPort = srcRtpPort;
}

std::string CallSession::getCallID() const
{
	return _callID;
}

std::shared_ptr<SipClient> CallSession::getSrc() const
{
	return _src;
}

std::shared_ptr<SipClient> CallSession::getDest() const
{
	return _dest;
}

CallSession::State CallSession::getState() const
{
	return _state;
}

std::string CallSession::getFromTag() const
{
    return _fromTag;
}

void CallSession::setFromTag(std::string fromTag)
{
    _fromTag = fromTag;
}

std::string CallSession::getToTag() const
{
    return _toTag;
}

void CallSession::setToTag(std::string toTag)
{
    _toTag = toTag;
}

std::shared_ptr<MediaSession> CallSession::getMediaSession()
{
    return _mediaSession;
}

void CallSession::setMediaSession(std::shared_ptr<MediaSession> mediaSession)
{
    _mediaSession = mediaSession;
}

std::string CallSession::toString()
{
    return "CallID: " + _callID + " Src: " + _src->getNumber() + " Dest: " + _dest->getNumber();
}

uint32_t CallSession::getSrcRtpPort() const
{
	return _srcRtpPort;
}

uint32_t CallSession::getDestRtpPort() const
{
	return _destRtpPort;
}
