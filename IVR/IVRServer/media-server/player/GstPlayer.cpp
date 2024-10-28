#include "GstPlayer.h"
#include <gst/gst.h>
#include <spdlog/spdlog.h>

static bool initialized = false;

GstPlayer::GstPlayer() :  _pipeline(nullptr) {
    if (!initialized) {
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

void GstPlayer::open()
{
    if (_rtpPort == 0 || _rtpHost.empty()) {
        spdlog::error("RTP host or port not set");
        return;
    }

    if (_pipeline) {
        initPipeline();
    }
}

void GstPlayer::start()
{
    spdlog::info("Starting player ...");
}

void GstPlayer::stop()
{

}

void GstPlayer::close()
{
    if (_pipeline) {
        gst_element_set_state(_pipeline, GST_STATE_NULL);
        gst_object_unref(_pipeline);
        _pipeline = nullptr;
    }
}

void GstPlayer::initPipeline()
{

}



GstPlayer::~GstPlayer()
{
    close();
}
