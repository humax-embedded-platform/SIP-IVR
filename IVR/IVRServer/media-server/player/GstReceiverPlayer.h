#ifndef GSTRECEIVERPLAYER_H
#define GSTRECEIVERPLAYER_H

#include "GstBasePlayer.h"

class GstReceiverPlayer : public GstBasePlayer
{
public:
    GstReceiverPlayer();

    void initPipeline() override;
    void destroyPipeline() override;

    gpointer onPlayerThreadStarted(gpointer data) override;
    gboolean onBusCallback(GstBus *bus, GstMessage *message, gpointer data) override;

    std::string getDTMFEvent();
    void setDTMFEvent(const std::string &dtmfEvent);

private:
    static void onPadAdded (GstElement* object, GstPad* new_pad, gpointer user_data);
    static void onNewSSrc (GstElement* object, guint arg0, guint arg1, gpointer user_data);
    static GstCaps *onRequestPtMap (GstElement* object, guint arg0, guint payload, gpointer user_data);

private:
    std::string _dtmfEvent;
};

#endif // GSTRECEIVERPLAYER_H
