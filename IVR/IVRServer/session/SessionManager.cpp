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

std::shared_ptr<CallSession> SessionManager::getSession(std::string callID)
{
    if (_sessionMap.find(callID) != _sessionMap.end())
    {
        return _sessionMap[callID];
    }
    return nullptr;
}

std::shared_ptr<CallSession> SessionManager::createSession(std::string callerNumber, std::string callID)
{
    LOG_D << "Creating session for caller: " << callerNumber << " callID: " << callID << ENDL;
    if (_sessionMap.find(callerNumber) == _sessionMap.end())
    {
        std::shared_ptr<CallSession> session(new CallSession(callID));
        _sessionMap[callerNumber] = session;
        return session;
    } else {
        LOG_E << "Session already exists" << ENDL;
        return nullptr;
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
