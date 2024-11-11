#include "MediaManager.h"
#include "MediaSession.h"
#include "MediaClient.h"
#include "Log.hpp"

namespace gstivr {

static MediaManager *instance = nullptr;

MediaManager::MediaManager() {
    _mediaClient = std::make_shared<MediaClient>();
}

MediaManager *MediaManager::getInstance()
{
    if (instance == nullptr)
    {
        instance = new MediaManager();
    }
    return instance;
}

std::shared_ptr<MediaSession> MediaManager::createSession(std::string clientIp, int clientRtpPort, std::string mediaDesc)
{
    Logger::getLogger()->info("Creating media session: {}:{}", clientIp, clientRtpPort);
    std::shared_ptr<MediaSession> mediaSession(new MediaSession(clientIp, clientRtpPort, mediaDesc));
    if (_mediaClient->initSession(mediaSession)) {
        _sessionMap[mediaSession->getSessionID()] = mediaSession;
        return mediaSession;
    }
    return nullptr;
}

void MediaManager::removeSession(std::shared_ptr<MediaSession> mediaSession)
{
    if (mediaSession )
    {
        std::string sessionID = mediaSession->getSessionID();
        Logger::getLogger()->info("Removing session: {}", sessionID);
        if (_sessionMap.find(sessionID) != _sessionMap.end())
        {
            _mediaClient->closeSession(mediaSession);
            _sessionMap.erase(sessionID);
        }
    }
}

bool MediaManager::startMediaSession(std::shared_ptr<MediaSession> mediaSession)
{
    Logger::getLogger()->info("Starting media session: {}:{}", mediaSession->remoteHost(), mediaSession->remotePort());
    _mediaClient->startSession(mediaSession);
    mediaSession->startReadDTMF(_mediaClient);
    return true;
}

bool MediaManager::stopMediaSession(std::shared_ptr<MediaSession> mediaSession)
{
    Logger::getLogger()->info("Stopping media session: {}:{}", mediaSession->remoteHost(), mediaSession->remotePort());
    mediaSession->stopReadDTMF();
    _mediaClient->stopSession(mediaSession);
    return true;
}

bool MediaManager::updateMediaSession(std::shared_ptr<MediaSession> mediaSession)
{
    Logger::getLogger()->info("Updating media session: {}:{}", mediaSession->remoteHost(), mediaSession->remotePort());
    _mediaClient->updateSession(mediaSession);
    return true;
}
} // namespace gstivr

