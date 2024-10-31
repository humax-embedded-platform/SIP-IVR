#ifndef GSTSENDERPLAYER_H
#define GSTSENDERPLAYER_H

#include "GstBasePlayer.h"

class GstSenderPlayer : public GstBasePlayer
{
public:
    explicit GstSenderPlayer();
    ~GstSenderPlayer();

    void initPipeline() override;
    void destroyPipeline() override;
    void setPBSourceFile(const std::string &sourceFile);

    gpointer onPlayerThreadStarted(gpointer data) override;
    gboolean onBusCallback(GstBus *bus, GstMessage *message, gpointer data) override;
};

#endif // GSTSENDERPLAYER_H
