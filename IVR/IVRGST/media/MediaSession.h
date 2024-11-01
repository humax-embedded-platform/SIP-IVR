#ifndef MEDIASESSION_H
#define MEDIASESSION_H

#include <string>
#include "Log.hpp"
#include <memory>
#include <thread>
#include <mutex>

class MediaSession;
class MediaClient;

class MediaSessionCallback
{
public:
    explicit MediaSessionCallback() = default;

    virtual void onDTMFEvent(std::shared_ptr<MediaSession> session, std::string event) = 0;
};

class MediaSession : public std::enable_shared_from_this<MediaSession>
{
public:
    explicit MediaSession(std::string clientIp, int clientRtpPort, std::string mediaDesc);
    ~MediaSession();

    std::string remoteHost();
    int remotePort();

    std::string getSessionID() const;
    void setSessionID(std::string sessionID);

    std::string getPbSourceFile() const;
    void setPbSourceFile(std::string pbSourceFile);

    void setMediaDescription(std::string mediaDescription);
    std::string getMediaDescription() const;

    void setMediaSessionCallback(std::shared_ptr<MediaSessionCallback> callback);

    void startReadDTMF(std::shared_ptr<MediaClient> mediaClient);
    void stopReadDTMF();
private:
    std::string _clientIp;
    int _clientRtpPort;
    std::string _sessionID;
    std::string _pbSourceFile;
    std::string _mediaDes;
    std::shared_ptr<MediaSessionCallback> _callback;
    std::shared_ptr<std::thread> _readDTMFThread;
    bool _readDTMFThreadRunning;
};

#endif // MEDIASESSION_H
