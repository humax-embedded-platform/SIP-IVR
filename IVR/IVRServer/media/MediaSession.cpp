#include "MediaSession.h"

MediaSession::MediaSession(std::string clientIp, int clientRtpPort, std::string mediaDesc) :
    _clientIp(clientIp),
    _clientRtpPort(clientRtpPort),
    _mediaDes(mediaDesc)
{
    LOG_I << "MediaSession created for client: " << clientIp << ":" << clientRtpPort << ENDL;
}

std::string MediaSession::remoteHost()
{
    return _clientIp;
}

int MediaSession::remotePort()
{
    return _clientRtpPort;
}

std::string MediaSession::getSessionID() const
{
    return _sessionID;
}

void MediaSession::setSessionID(std::string sessionID)
{
    _sessionID = sessionID;
}

std::string MediaSession::getPbSourceFile() const
{
    return _pbSourceFile;
}

void MediaSession::setPbSourceFile(std::string pbSourceFile)
{
    _pbSourceFile = pbSourceFile;
}


void MediaSession::setMediaDescription(std::string mediaDescription) {
    _mediaDes = mediaDescription;
}

std::string MediaSession::getMediaDescription() const
{
    return _mediaDes;
}
