#ifndef GSTSENDERPLAYER_H
#define GSTSENDERPLAYER_H

#include "GstBasePlayer.h"

class GstSenderPlayer : public GstBasePlayer
{
public:
    GstSenderPlayer();

    void initPipeline() override;
    void destroyPipeline() override;

};

#endif // GSTSENDERPLAYER_H
