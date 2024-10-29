#ifndef MEDIASESSION_H
#define MEDIASESSION_H

#include <string>
#include "Log.hpp"

class MediaSession
{
public:
    explicit MediaSession(std::string clientIp, int clientRtpPort, std::string mediaDesc);

    std::string remoteHost();
    int remotePort();

    std::string getSessionID() const;
    void setSessionID(std::string sessionID);

    std::string getPbSourceFile() const;
    void setPbSourceFile(std::string pbSourceFile);

    void setMediaDescription(std::string mediaDescription);
    std::string getMediaDescription() const;

private:
    std::string _clientIp;
    int _clientRtpPort;
    std::string _sessionID;
    std::string _pbSourceFile;
    std::string _mediaDes;
};

#endif // MEDIASESSION_H
