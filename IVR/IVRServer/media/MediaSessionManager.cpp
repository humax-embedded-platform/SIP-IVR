#include "MediaSessionManager.h"

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

std::shared_ptr<MediaSession> MediaSessionManager::createSession()
{
    return nullptr;
}
