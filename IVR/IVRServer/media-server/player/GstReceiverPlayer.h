#ifndef GSTRECEIVERPLAYER_H
#define GSTRECEIVERPLAYER_H

#include "GstBasePlayer.h"

class GstReceiverPlayer : public GstBasePlayer
{
public:
    GstReceiverPlayer();

    void initPipeline() override;
};

#endif // GSTRECEIVERPLAYER_H
