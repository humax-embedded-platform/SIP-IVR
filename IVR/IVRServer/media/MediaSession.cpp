#include "MediaSession.h"

MediaSession::MediaSession(std::string clientIp, int clientRtpPort) :
    _clientIp(clientIp),
    _clientRtpPort(clientRtpPort)
{
    LOG_I << "MediaSession created for client: " << clientIp << ":" << clientRtpPort << ENDL;
}

std::string MediaSession::getSessionID() const
{
    return generateSessionID(_clientIp, _clientRtpPort);
}

std::string MediaSession::generateSessionID(std::string clientIp, int clientRtpPort)
{
    return clientIp + ":" + std::to_string(clientRtpPort);
}
