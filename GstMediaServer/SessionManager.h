#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <map>
#include <memory>

#define MAX_SESSIONS 5
#define START_PORT 60000 // up to 65535

class MediaSession;

class SessionManager : public std::enable_shared_from_this<SessionManager>
{
public:
    static std::shared_ptr<SessionManager> getInstance();
    bool exceedsMaxSessions();
    std::shared_ptr<MediaSession> createSession(std::string remoteHost, int remotePort);
    std::shared_ptr<MediaSession> getSession(std::string sessionID);
    void removeSession(std::string sessionID);

private:
    int getAvailablePort();
    bool remotePortInUse(int port);

private:
    std::map<int, std::shared_ptr<MediaSession>> _sessions;
};

#endif // SESSIONMANAGER_H
