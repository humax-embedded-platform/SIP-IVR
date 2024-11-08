#include "SessionManager.h"
#include "util/Log.hpp"
#include "MediaSession.h"

static std::shared_ptr<SessionManager> _instance = nullptr;


std::shared_ptr<SessionManager> SessionManager::getInstance()
{
    if (_instance == nullptr)
    {
        _instance = std::make_shared<SessionManager>();
    }
    return _instance;
}

bool SessionManager::exceedsMaxSessions()
{
    return _sessions.size() >= MAX_SESSIONS;
}

std::shared_ptr<MediaSession> SessionManager::createSession(std::string remoteHost, int remotePort)
{
    if (exceedsMaxSessions())
    {
        Logger::getLogger()->error("Max sessions reached");
        return nullptr;
    }

    if (remotePortInUse(remotePort))
    {
        Logger::getLogger()->error("Remote port already in use");
        return nullptr;
    }

    int localPort = getAvailablePort();
    if (localPort == -1)
    {
        return nullptr;
    }

    auto session = std::make_shared<MediaSession>(remoteHost, remotePort, localPort);
    _sessions[localPort] = session;
    return session;
}

int SessionManager::getAvailablePort()
{
    for (int port = START_PORT; port < 65536; port++)
    {
        if (_sessions.find(port) == _sessions.end())
        {
            return port;
        }
    }
    return -1;
}

bool SessionManager::remotePortInUse(int port)
{
    for (auto& [localPort, session] : _sessions)
    {
        if (session->remotePort() == port)
        {
            return true;
        }
    }
    return false;
}

std::shared_ptr<MediaSession> SessionManager::getSession(std::string sessionID)
{
    for (auto& [localPort, session] : _sessions)
    {
        if (session->sessionID() == sessionID)
        {
            return session;
        }
    }
    return nullptr;
}

void SessionManager::removeSession(std::string sessionID)
{
    for (auto it = _sessions.begin(); it != _sessions.end(); it++)
    {
        if (it->second->sessionID() == sessionID)
        {
            _sessions.erase(it);
            return;
        }
    }
}
