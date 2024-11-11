#include "MediaSession.h"
#include "MediaClient.h"
#include "filesystem"
#include "Log.hpp"

namespace gstivr {

MediaSession::MediaSession(std::string clientIp, int clientRtpPort, std::string mediaDesc) :
    _clientIp(clientIp),
    _clientRtpPort(clientRtpPort),
    _mediaDes(mediaDesc),
    _callback(nullptr),
    _readDTMFThreadRunning(false)
{
    Logger::getLogger()->info("MediaSession created for client: {}:{}", clientIp, clientRtpPort);
}

MediaSession::~MediaSession()
{
    _readDTMFThreadRunning = false;
    if (_readDTMFThread) {
        _readDTMFThread->join();
    }
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
    // check if file exists
    if (!std::filesystem::exists(pbSourceFile))
    {
        Logger::getLogger()->error("File does not exist: {}", pbSourceFile);
        return;
    }
    _pbSourceFile = pbSourceFile;
}


void MediaSession::setMediaDescription(std::string mediaDescription) {
    _mediaDes = mediaDescription;
}

std::string MediaSession::getMediaDescription() const
{
    return _mediaDes;
}

void MediaSession::setMediaSessionCallback(std::shared_ptr<MediaSessionCallback> callback)
{
    _callback = callback;
}

void MediaSession::startReadDTMF(std::shared_ptr<MediaClient> mediaClient)
{
    _readDTMFThreadRunning = true;
    _readDTMFThread = std::make_shared<std::thread>([this, mediaClient] {
        while (_readDTMFThreadRunning)
        {
            std::string dtmf = mediaClient->readDTMF(shared_from_this());
            if (dtmf.length() > 0)
            {
                Logger::getLogger()->info("DTMF: {}", dtmf);
                if (_callback)
                {
                    _callback->onDTMFEvent(shared_from_this(), dtmf);
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
}

void MediaSession::stopReadDTMF()
{
    _readDTMFThreadRunning = false;
    if (_readDTMFThread) {
        _readDTMFThread->join();
        _readDTMFThread = nullptr;
    }
}
} // namespace gstivr
