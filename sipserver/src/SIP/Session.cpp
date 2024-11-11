#include "Session.hpp"
#include "Log.hpp"

namespace sipserver {

Session::Session(std::string callID, std::shared_ptr<SipClient> src) :
    _callID(std::move(callID)), _src(src), _state(State::Invited)
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

void Session::setDest(std::shared_ptr<SipClient> dest)
{
	_dest = dest;
}

void Session::setReferedDest(std::shared_ptr<SipClient> dest)
{
    _referedDest = dest;
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
} // namespace sipserver
