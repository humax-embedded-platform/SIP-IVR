#ifndef MEDIAMANAGER_H
#define MEDIAMANAGER_H

#include <map>
#include <memory>

class MediaClient;
class MediaSession;

class MediaManager
{
private:
    MediaManager();

public:
    static MediaManager* getInstance();

    std::shared_ptr<MediaSession> createSession(std::string clientIp, int clientRtpPort, std::string mediaDesc);
    void removeSession(std::shared_ptr<MediaSession> mediaSession);

    bool startMediaSession(std::shared_ptr<MediaSession> mediaSession);
    bool stopMediaSession(std::shared_ptr<MediaSession> mediaSession);

private:
    std::map<std::string, std::shared_ptr<MediaSession>> _sessionMap;
    std::shared_ptr<MediaClient> _mediaClient;
};

#endif // MEDIAMANAGER_H
