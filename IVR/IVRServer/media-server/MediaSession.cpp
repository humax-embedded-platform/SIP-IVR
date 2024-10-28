#include "MediaSession.h"
#include "util/HashUtil.h"
#include "spdlog/spdlog.h"
#include "player/GstPlayer.h"

MediaSession::MediaSession(std::string remoteHost, int remotePort, int localPort):
    _remoteHost(remoteHost),
    _remotePort(remotePort),
    _localPort(localPort) {
    // generate session ID as md5 hash of remote host and port
    _sessionID = HashUtil::generateMD5(remoteHost + std::to_string(remotePort));

    _player = std::make_shared<GstPlayer>();
    _player->setRtpHost(remoteHost);
    _player->setRtpPort(remotePort);
    _player->open();

    spdlog::info("Created session with ID {}", _sessionID);
}

std::string MediaSession::sessionID() const
{
    return _sessionID;
}

std::string MediaSession::remoteHost() const
{
    return _remoteHost;
}

int MediaSession::remotePort() const
{
    return _remotePort;
}

int MediaSession::localPort() const
{
    return _localPort;
}

bool MediaSession::start()
{
    _player->start();
}

bool MediaSession::stop()
{
    _player->stop();
}
