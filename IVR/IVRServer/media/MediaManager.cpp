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
    std::string sessionID = MediaSession::generateSessionID(clientIp, clientRtpPort);
    LOG_D << "Creating session: " << sessionID << ENDL;
    if (_sessionMap.find(sessionID) != _sessionMap.end())
    {
        LOG_D << "Session already exists" << ENDL;
        return nullptr;
    }

    std::shared_ptr<MediaSession> mediaSession(new MediaSession(clientIp, clientRtpPort));
    _sessionMap[sessionID] = mediaSession;
    return mediaSession;
}

void MediaSessionManager::removeSession(std::shared_ptr<MediaSession> mediaSession)
{
    if (mediaSession )
    {
        std::string sessionID = mediaSession->getSessionID();
        LOG_D << "Removing session: " << sessionID << ENDL;
        if (_sessionMap.find(sessionID) != _sessionMap.end())
        {
            _sessionMap.erase(sessionID);
        }
    }
}

bool MediaSessionManager::startMediaSession(std::shared_ptr<MediaSession> mediaSession)
{
    LOG_D << "Starting media session" << ENDL;
    return true;
}

bool MediaSessionManager::stopMediaSession(std::shared_ptr<MediaSession> mediaSession)
{
    LOG_D << "Stopping media session" << ENDL;
    return true;
}
