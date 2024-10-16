#include "MediaSessionManager.h"
#include "MediaSession.h"

static MediaSessionManager *instance = nullptr;

MediaSessionManager::MediaSessionManager() {}

MediaSessionManager *MediaSessionManager::getInstance()
{
    if (instance == nullptr)
    {
        instance = new MediaSessionManager();
    }
    return instance;
}

std::shared_ptr<MediaSession> MediaSessionManager::createSession(std::string clientIp, int clientRtpPort)
{
    LOG_D << "Creating session for client: " << clientIp << ":" << clientRtpPort << ENDL;
    std::string sessionID = MediaSession::generateSessionID(clientIp, clientRtpPort);
    LOG_D << "SessionID: " << sessionID << ENDL;
    if (_sessionMap.find(sessionID) != _sessionMap.end())
    {
        LOG_D << "Session already exists" << ENDL;
        return nullptr;
    }
    LOG_D << "Creating new session" << ENDL;
    std::shared_ptr<MediaSession> mediaSession(new MediaSession(clientIp, clientRtpPort));
    _sessionMap[sessionID] = mediaSession;
    return mediaSession;
}
