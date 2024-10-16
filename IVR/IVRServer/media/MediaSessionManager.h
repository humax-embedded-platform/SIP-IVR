#ifndef MEDIASESSIONMANAGER_H
#define MEDIASESSIONMANAGER_H

#include <map>
#include <memory>

class MediaSession;

class MediaSessionManager
{
private:
    MediaSessionManager();

public:
    static MediaSessionManager* getInstance();

    std::shared_ptr<MediaSession> createSession();

private:
    std::map<std::string, std::shared_ptr<MediaSession>> _sessionMap;
};

#endif // MEDIASESSIONMANAGER_H
