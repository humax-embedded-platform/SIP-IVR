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

    std::shared_ptr<MediaSession> createSession(std::string clientIp, int clientRtpPort);
    void removeSession(std::shared_ptr<MediaSession> mediaSession);

    bool startMediaSession(std::shared_ptr<MediaSession> mediaSession);
    bool stopMediaSession(std::shared_ptr<MediaSession> mediaSession);

private:
    std::map<std::string, std::shared_ptr<MediaSession>> _sessionMap;
};

#endif // MEDIASESSIONMANAGER_H
