#ifndef MEDIASESSION_H
#define MEDIASESSION_H


#include <string>
#include <memory>

namespace gstmediaserver {

class GstPlayer;
class MediaSession
{
public:
    MediaSession(std::string remoteHost, int remotePort, int localPort);

    std::string sessionID() const;
    std::string remoteHost() const;
    int remotePort() const;
    int localPort() const;

    void setPBSourceFile(std::string sourceFile);
    void setMediaDescription(std::string sdp);

    bool open();
    bool start();
    bool stop();
    bool close();

    std::string getDTMFEvent();
    void clearDTMFEvent();

private:
    std::string _sessionID;
    int _localPort;
    std::string _remoteHost;
    int _remotePort;

    std::shared_ptr<GstPlayer> _player;
};

} // namespace gstmediaserver

#endif // MEDIASESSION_H
