#include "SessionManager.h"
#include "Log.hpp"
#include "CallSession.hpp"

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

std::shared_ptr<CallSession> SessionManager::createSession(std::string callerNumber, std::string callID)
{
    if (_sessionMap.find(callerNumber) == _sessionMap.end())
    {
        std::shared_ptr<CallSession> session(new CallSession(callID));
        _sessionMap[callID] = session;
        return session;
    }
}

void SessionManager::removeSession(std::string callID)
{
    if (_sessionMap.find(callID) != _sessionMap.end())
    {
        _sessionMap.erase(callID);
    }
}

void SessionManager::dumpSessions()
{
    for (auto &session : _sessionMap)
    {
        LOG_D << "Session: " << ENDL;
    }
}
