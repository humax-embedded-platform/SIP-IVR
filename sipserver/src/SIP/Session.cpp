#include "Session.hpp"
#include "Log.hpp"

namespace sipserver {

Session::Session(std::string callID, std::shared_ptr<SipClient> src, uint32_t srcRtpPort) :
    _callID(std::move(callID)), _src(src), _state(State::Invited), _srcRtpPort(srcRtpPort), _destRtpPort(0)
{
}

void Session::setState(State state)
{
	if (state == _state)
		return;
	_state = state;
	if (state == State::Connected)
    {
        Logger::getLogger()->info("Session Created between {} and {}", _src->getNumber(), _dest->getNumber());
	}
}

void Session::setDest(std::shared_ptr<SipClient> dest, uint32_t rtpPort)
{
    _dest = dest;
    _destRtpPort = rtpPort;
}

void Session::setReferedDest(std::shared_ptr<SipClient> dest, uint32_t rtpPort)
{
    _referedDest = dest;
    _referedDestRtpPort = rtpPort;
}

std::string Session::getCallID() const
{
	return _callID;
}

std::shared_ptr<SipClient> Session::getSrc() const
{
	return _src;
}

std::shared_ptr<SipClient> Session::getDest() const
{
	return _dest;
}

std::shared_ptr<SipClient> Session::getReferedDest() const
{
    return _referedDest;
}

Session::State Session::getState() const
{
	return _state;
}

std::string Session::curOriginTransaction() const
{
    return _curOriginTransaction;
}

void Session::setCurOriginTransaction(const std::string &transaction)
{
    _curOriginTransaction = transaction;
}

std::string Session::curReferedTransaction() const
{
    return _curReferedTransaction;
}

void Session::setCurReferedTransaction(const std::string &transaction)
{
    _curReferedTransaction = transaction;
}

uint32_t Session::getSrcRtpPort() const
{
    return _srcRtpPort;
}

uint32_t Session::getDestRtpPort() const
{
    return _destRtpPort;
}

uint32_t Session::getReferedDestRtpPort() const
{
    return _referedDestRtpPort;
}

} // namespace sipserver