#include "MediaManager.h"
#include "MediaSession.h"
#include "MediaClient.h"

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
    LOG_D << "Creating media session: " << clientIp << ":" << clientRtpPort << ENDL;
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
        LOG_D << "Removing session: " << sessionID << ENDL;
        if (_sessionMap.find(sessionID) != _sessionMap.end())
        {
            _mediaClient->closeSession(mediaSession);
            _sessionMap.erase(sessionID);
        }
    }
}

bool MediaManager::startMediaSession(std::shared_ptr<MediaSession> mediaSession)
{
    LOG_D << "Starting media session" << ENDL;
    _mediaClient->startSession(mediaSession);
    mediaSession->startReadDTMF(_mediaClient);
    return true;
}

bool MediaManager::stopMediaSession(std::shared_ptr<MediaSession> mediaSession)
{
    LOG_D << "Stopping media session" << ENDL;
    _mediaClient->stopSession(mediaSession);
    return true;
}

bool MediaManager::updateMediaSession(std::shared_ptr<MediaSession> mediaSession)
{
    LOG_D << "Updating media session" << ENDL;
    _mediaClient->updateSession(mediaSession);
    return true;
}
