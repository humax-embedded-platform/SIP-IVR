#ifndef MEDIASESSION_H
#define MEDIASESSION_H

#include <string>
#include "Log.hpp"

class MediaSession
{
public:
    explicit MediaSession(std::string clientIp, int clientRtpPort);

    std::string getSessionID() const;
    static std::string generateSessionID(std::string clientIp, int clientRtpPort);
private:
    std::string _clientIp;
    int _clientRtpPort;
};

#endif // MEDIASESSION_H
