#ifndef MEDIASESSION_H
#define MEDIASESSION_H

#include <string>
#include <memory>

class GstPlayer;
class MediaSession
{
public:
    MediaSession(std::string remoteHost, int remotePort, int localPort);

    std::string sessionID() const;
    std::string remoteHost() const;
    int remotePort() const;
    int localPort() const;

    bool start();
    bool stop();
private:
    std::string _sessionID;
    int _localPort;
    std::string _remoteHost;
    int _remotePort;

    std::shared_ptr<GstPlayer> _player;
};

#endif // MEDIASESSION_H
