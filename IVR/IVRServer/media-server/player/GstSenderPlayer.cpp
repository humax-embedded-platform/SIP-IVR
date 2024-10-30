#include "GstSenderPlayer.h"

GstSenderPlayer::GstSenderPlayer() : GstBasePlayer() {}

void GstSenderPlayer::initPipeline()
{
    spdlog::info("Initializing pipeline");
}

void GstSenderPlayer::destroyPipeline()
{
    spdlog::info("Destroying pipeline");
}

bool GstSenderPlayer::open(const std::string &uri)
{
    GstBasePlayer::open();
}

bool GstSenderPlayer::start()
{

}

bool GstSenderPlayer::stop()
{

}

bool GstSenderPlayer::close()
{

}
