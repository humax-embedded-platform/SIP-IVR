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
    _player->setRtpHost(_remoteHost);
    _player->setRtpPort(_remotePort);
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

void MediaSession::setPBSourceFile(std::string sourceFile)
{
    _player->setPBSourceFile(sourceFile);
}

void MediaSession::setMediaDescription(std::string sdp)
{
    // if spd start with rtpmap: extract payload type, codec and sample rate
    if (sdp.find("rtpmap:") != std::string::npos) {
        auto rtpmap = sdp.substr(sdp.find("rtpmap:") + 7);
        auto payloadType = rtpmap.substr(0, rtpmap.find(" "));
        rtpmap = rtpmap.substr(rtpmap.find(" ") + 1);
        auto codec = rtpmap.substr(0, rtpmap.find("/"));
        rtpmap = rtpmap.substr(rtpmap.find("/") + 1);
        auto sampleRate = rtpmap.substr(0, rtpmap.find("/"));

        _player->setPayloadType(std::stoi(payloadType));
        _player->setCodec(codec);
        _player->setSampleRate(std::stoi(sampleRate));
        spdlog::info("Media description set: payloadType: {}, codec: {}, sampleRate: {}", payloadType, codec, sampleRate);
    }
}

bool MediaSession::open()
{
    return _player->open();
}

bool MediaSession::start()
{
    return _player->start();
}

bool MediaSession::stop()
{
    return _player->stop();
}

bool MediaSession::close()
{
    return _player->close();
}
