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
    void setLaunchCmd(const std::string &launchCmd);
    void setPBSourceFile(const std::string &sourceFile);

    gpointer onPlayerThreadStarted(gpointer data) override;
    gboolean onBusCallback(GstBus *bus, GstMessage *message, gpointer data) override;
private:
    std::string _launchCmd;
};

#endif // GSTSENDERPLAYER_H
