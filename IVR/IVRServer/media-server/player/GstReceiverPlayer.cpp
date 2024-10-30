#include "GstReceiverPlayer.h"

GstReceiverPlayer::GstReceiverPlayer() : GstBasePlayer() {

}

void GstReceiverPlayer::initPipeline()
{

}

void GstReceiverPlayer::destroyPipeline()
{

}

gpointer GstReceiverPlayer::onPlayerThreadStarted(gpointer data)
{
    spdlog::info("Player thread started");
}

gboolean GstReceiverPlayer::onBusCallback(GstBus *bus, GstMessage *message, gpointer data)
{
    spdlog::info("Bus callback");
}
