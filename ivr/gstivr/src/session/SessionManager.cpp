#include "SessionManager.h"
#include "Log.hpp"
#include "CallSession.hpp"

namespace gstivr {

static SessionManager *instance = nullptr;

SessionManager::SessionManager() {
    _sessionMap.clear();
}

SessionManager *SessionManager::getInstance()
{
    if (instance == nullptr)
    {
        instance = new SessionManager();
    }
    return instance;
}

std::shared_ptr<CallSession> SessionManager::getSession(std::string callID)
{
    if (_sessionMap.find(callID) != _sessionMap.end())
    {
        return _sessionMap[callID];
    }
    return nullptr;
}

std::shared_ptr<CallSession> SessionManager::createSession(std::string callID)
{
    Logger::getLogger()->info("Creating session for callID: {}", callID);
    if (_sessionMap.find(callID) == _sessionMap.end())
    {
        std::shared_ptr<CallSession> session(new CallSession(callID));
        _sessionMap[callID] = session;
        return session;
    } else {
        Logger::getLogger()->error("Session already exists");
        return nullptr;
    }
}

std::shared_ptr<CallSession> SessionManager::findSession(std::shared_ptr<MediaSession> mediaSession)
{
    for (auto &session : _sessionMap)
    {
        if (session.second->getMediaSession() == mediaSession)
        {
            return session.second;
        }
    }
    return nullptr;
}

void SessionManager::removeSession(std::string callID)
{
    if (_sessionMap.find(callID) != _sessionMap.end())
    {
        _sessionMap.erase(callID);
    }
}

void SessionManager::removeSession(std::shared_ptr<CallSession> session)
{
    removeSession(session->getCallID());
}


void SessionManager::dumpSessions()
{
    // do nothing
}
} // namespace gstivr

