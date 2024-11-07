#include "GstPlayer.h"
#include <gst/gst.h>
#include <spdlog/spdlog.h>
#include "util/FileUtil.h"
#include "GstSenderPlayer.h"
#include "GstReceiverPlayer.h"

static bool initialized = false;

GstPlayer::GstPlayer() :
    _rtpPort(0),
    _rtpHost(""),
    _codec(""),
    _payloadType(0),
    _sampleRate(0),
    _pbSourceFile(""),
    _senderPlayer(nullptr),
    _receiverPlayer(nullptr) {
    if (!initialized) {
#if 0
        g_setenv("GST_DEBUG", "4", 1);
#endif
        gst_init(nullptr, nullptr);
        initialized = true;
    }
}

void GstPlayer::setRtpPort(int port)
{
    _rtpPort = port;
}

void GstPlayer::setRtpHost(std::string host)
{
    _rtpHost = host;
}

void GstPlayer::setCodec(std::string codec)
{
    spdlog::info("Setting codec: {}", codec);
    _codec = codec;
}

void GstPlayer::setPayloadType(int payloadType)
{
    spdlog::info("Setting payload type: {}", payloadType);
    _payloadType = payloadType;
}

void GstPlayer::setSampleRate(int sampleRate)
{
    spdlog::info("Setting sample rate: {}", sampleRate);
    _sampleRate = sampleRate;
}

void GstPlayer::setPBSourceFile(std::string sourceFile)
{
    if (sourceFile.empty()) {
        spdlog::error("Source file is empty");
        return;
    }

    auto absPath = FileUtil::absolutePath(sourceFile);
    spdlog::info("Setting playback source file: {}", absPath);

    _pbSourceFile = absPath;

    if (_senderPlayer) {
        _senderPlayer->setPBSourceFile(_pbSourceFile);
    }
}

bool GstPlayer::open()
{
    if (_rtpPort == 0 || _rtpHost.empty()) {
        spdlog::error("RTP host or port not set");
        return false;
    }

    initPlayer();

    if (_senderPlayer) {
        _senderPlayer->open();
    }
    if (_receiverPlayer) {
        _receiverPlayer->open();
    }
    return true;
}

bool GstPlayer::start()
{
    spdlog::info("Starting player ...");
    if (_senderPlayer) {
        _senderPlayer->start();
    }
    if (_receiverPlayer) {
        _receiverPlayer->start();
    }
    return true;
}

bool GstPlayer::stop()
{
    if (_senderPlayer) {
        _senderPlayer->stop();
    }
    if (_receiverPlayer) {
        _receiverPlayer->stop();
    }
    return true;
}

bool GstPlayer::close()
{
    if (_senderPlayer) {
        _senderPlayer->close();
    }
    if (_receiverPlayer) {
        _receiverPlayer->close();
    }
    return true;
}

std::string GstPlayer::getDTMFEvent()
{
    if (_receiverPlayer) {
        return _receiverPlayer->getDTMFEvent();
    }
    return "";
}

void GstPlayer::clearDTMFEvent()
{
    if (_receiverPlayer) {
        _receiverPlayer->setDTMFEvent("");
    }
}

void GstPlayer::initPlayer()
{
    spdlog::info("initPlayer");
    if (!_senderPlayer) {
        std::string cmd;

        // check parameters is missing
        if (_codec.empty() || _payloadType == 0 || _sampleRate == 0) {
            spdlog::error("Missing parameters");
            return;
        }

        if (_codec == "opus") {
            cmd = std::string("filesrc name=source") + _pbSourceFile + " ! wavparse ! audioconvert ! opusenc ! rtpopuspay pt=" +
                  std::to_string(_payloadType) + " ! udpsink host=" + _rtpHost + " port=" + std::to_string(_rtpPort);
        } else if (_codec == "speex") {
            cmd = std::string("filesrc name=source") + _pbSourceFile + " ! wavparse ! audioconvert ! audioresample ! audio/x-raw,rate=" + std::to_string(_sampleRate) + " ! speexenc ! rtpspeexpay pt=" +
                  std::to_string(_payloadType) + " ! udpsink host=" + _rtpHost + " port=" + std::to_string(_rtpPort);
        } else {
            spdlog::error("Unsupported codec");
            return;
        }

        spdlog::info("GstSenderPlayer command: {}", cmd);
        _senderPlayer = std::make_shared<GstSenderPlayer>(_rtpHost, _rtpPort);
        _senderPlayer->setLaunchCmd(cmd);
    }

    if (!_receiverPlayer) {
        _receiverPlayer = std::make_shared<GstReceiverPlayer>();
        std::string cmd = "rtpbin name=rtpbin  udpsrc name=udpsrc caps=application/x-rtp !  rtpbin.recv_rtp_sink_0 rtpbin.  rtpdtmfdepay name=rtpdtmfdepay ! fakesink name=fakesink";
        spdlog::info("GstReceiverPlayer command: {}", cmd);
        _receiverPlayer->setLaunchCmd(cmd);
    }
}



GstPlayer::~GstPlayer()
{
    close();
}
